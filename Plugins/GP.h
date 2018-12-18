#pragma once

#include <iostream>
#include <fstream>
#include "Eigen/Dense"
#include "json11/json11.hpp"

using namespace std;
using namespace Eigen;

// 学習データは平均０、分散１に正規化し、各関節は統計的に独立を仮定する（これはよくないと思うが、将来的な課題とする）

class GP
{

  public:
    int dim, x_dim, y_dim;
    // 教師データ(Active set)の数
    int N;

    // 以下3つのパラメータはモデルの滑らかさに影響する。最適化計算中に変更することを検討する
    double kernel_variance; // = α
    // kernel一つ一つにlengthscaleを割り当てることも検討する
    double kernel_lengthscale; // = γ^{-0.5}
    double gaussian_variance; // = β^{-1}

    double smooth_kernel_variance;
    double smooth_kernel_lengthscale;
    double smooth_gaussian_variance;

    // カーネル行列とその逆行列
    MatrixXd K, K_inv;
    // カーネル行列は正定値対称行列なので、コレスキー分解ができる。その結果の下三角行列
    MatrixXd L;
    // 教師データ行列(N, y_dim)
    MatrixXd Y;
    // alpha=L*Y=Y^T * K^-1 事後確率を求めるときに簡単になる
    MatrixXd alpha;
    // 新しい潜在変数xと、教師データから得られるカーネル関数のベクトル
    VectorXd k_star;

    // 正規化する前の元のデータの平均、標準偏差、分散
    VectorXd mean, st, va;

    std::vector<VectorXd *> inputs;

    // RBFカーネルはこれで間違いないはず
    inline double rbf(const VectorXd &x1, const VectorXd &x2)
    {
        return kernel_variance * exp(-0.5 * (x1 - x2).squaredNorm() / (kernel_lengthscale * kernel_lengthscale));
    }

    // GPyで生成したパラメータの読み込み。
    void load(const char *filename)
    {
        ifstream ifs(filename);
        istreambuf_iterator<char> it(ifs);
        istreambuf_iterator<char> last;
        string json_str(it, last);
        string err;

        auto json = json11::Json::parse(json_str, err);
        auto &arrX = json["X"].array_items();
        auto &arrY = json["Y"].array_items();
        auto &arrMean = json["mean"].array_items();
        auto &arrStd = json["std"].array_items();

        N = arrX.size();
        x_dim = arrX[0].array_items().size();
        y_dim = arrY[0].array_items().size();
        dim = x_dim + y_dim;

        Y.resize(N, y_dim);

        inputs.clear();
        for (int i = 0; i < N; i++)
        {
            VectorXd *x = new VectorXd(x_dim);

            for (int ix = 0; ix < x_dim; ix++)
                (*x)(ix) = arrX[i][ix].number_value();

            for (int iy = 0; iy < y_dim; iy++)
                Y(i, iy) = arrY[i][iy].number_value();

            inputs.push_back(x);
        }

        mean.resize(y_dim);
        st.resize(y_dim);
        va.resize(y_dim);
        for (int i = 0; i < y_dim; i++)
        {
            mean(i) = arrMean[i].number_value();
            st(i) = arrStd[i].number_value();
        }
        va.noalias() = st.cwiseProduct(st);

        kernel_variance = json["kernel"]["variance"][0].number_value();
        kernel_lengthscale = json["kernel"]["lengthscale"][0].number_value();
        gaussian_variance = json["likelihood"]["variance"][0].number_value();

        // 共分散行列（カーネル行列）の計算
        // 対称行列なので下半分だけ求めればよいはずー＞これからやる
        K.resize(N, N);
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                K(i, j) = rbf((*inputs[i]), (*inputs[j]));
            }
        }

        MatrixXd Ky = K;
        Ky.diagonal() += (gaussian_variance + 1e-8) * VectorXd::Ones(N);

        L.resize(N, N);
        L.topLeftCorner(N, N) = Ky.topLeftCorner(N, N).selfadjointView<Eigen::Lower>().llt().matrixL();

        alpha.resize(N, y_dim);
        alpha = L.topLeftCorner(N, N).triangularView<Lower>().solve(Y);
        L.topLeftCorner(N, N).triangularView<Eigen::Lower>().adjoint().solveInPlace(alpha);
        // alpha = Ky.llt().solve(Y);

        // 逆行列
        K_inv = L * L.transpose();
        K_inv = K_inv.inverse();

        k_star.resize(N);

        ifs.close();

    }

    void update_k_star(const VectorXd &x_star)
    {
        for (int i = 0; i < N; i++)
        {
            k_star(i) = rbf(x_star, (*inputs[i]));
        }
    }

    inline VectorXd f()
    {
        return (k_star.transpose() * K_inv * Y).transpose() + mean;
        // return (alpha.transpose() * k_star) + mean;
    }

    inline double sigma(const VectorXd &x)
    {
        VectorXd v = L.topLeftCorner(N, N).triangularView<Eigen::Lower>().solve(k_star);
        return rbf(x, x) - v.dot(v) + gaussian_variance;
    }

    inline MatrixXd dk_stardx(const VectorXd &x){
        MatrixXd _dk_stardx;
        _dk_stardx.resize(N, x_dim);
        for (int ix = 0; ix < N; ix++)
        {
            _dk_stardx.row(ix) = ((*inputs[ix]) - x) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }
        return _dk_stardx;
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        VectorXd _x = x.tail(x_dim);
        VectorXd _y = x.head(y_dim);

        update_k_star(_x);

        // 事後分布の分散
        double _sigma = sigma(_x);

        // assert(_sigma > 0)

        // 事後分布の平均
        VectorXd _f = f();

        // yの勾配
        VectorXd y_grad = (_y - _f).cwiseProduct(va) / _sigma;

        // p(y|x)と相似な値
        double pyx = (_y - _f).cwiseProduct(st).squaredNorm() / _sigma;

        // Shape: (N, dim_x)
        MatrixXd _dk_stardx = dk_stardx(_x);

        // Shape: (dim_y, dim_x)
        MatrixXd _dfdx = alpha.transpose() * _dk_stardx;

        // (dim_x)
        VectorXd _dsigmadx = -2. * k_star.transpose() * K_inv * _dk_stardx;

        // xの勾配
        x_grad.tail(x_dim) = -_dfdx.transpose() * y_grad + _dsigmadx * (y_dim - pyx) / (2. * _sigma) + _x;
        x_grad.head(y_dim) = y_grad;

        // 尤度
        return (pyx + (y_dim * log(_sigma)) + _x.squaredNorm()) / 2.;
    }
};