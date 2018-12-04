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
    double kernel_variance;
    // kernel一つ一つにlengthscaleを割り当てることも検討する
    double kernel_lengthscale;
    double gaussian_variance;

    // カーネル行列とその逆行列
    MatrixXd K, K_inv;
    // カーネル行列は正定値対称行列なので、コレスキー分解ができる。その結果の下三角行列
    MatrixXd L;
    // 教師データ行列(N, y_dim)
    MatrixXd Y;
    // alpha=L*Y 事後確率を求めるときに簡単になる
    MatrixXd alpha;
    // 新しい潜在変数xと、教師データから得られるカーネル関数のベクトル
    VectorXd k_star;

    // 正規化する前の元のデータの平均、分散
    VectorXd mean, st;

    std::vector<VectorXd *> inputs;

    // RBFカーネルはこれで間違いないはず
    double rbf(const VectorXd &x1, const VectorXd &x2)
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

        mean.resize(y_dim);
        st.resize(y_dim);

        for (int i = 0; i < N; i++)
        {
            VectorXd *x = new VectorXd(x_dim);

            for (int ix = 0; ix < x_dim; ix++)
                (*x)(ix) = arrX[i][ix].number_value();

            for (int iy = 0; iy < y_dim; iy++)
                Y(i, iy) = arrY[i][iy].number_value();

            inputs.push_back(x);

            st(i) = arrStd[i].number_value();
        }

        for (int i = 0; i < y_dim; i++)
        {
            mean(i) = arrMean[i].number_value();
            st(i) = arrStd[i].number_value();
        }

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
        
        // cout << K_inv << endl;
    }

    void update_k_star(const VectorXd &x_star)
    {
        for (int i = 0; i < N; i++)
        {
            k_star(i) = rbf(x_star, (*inputs[i]));
        }
    }

    // 事後分布の平均
    VectorXd f(const VectorXd &x)
    {
        VectorXd _f = (alpha.transpose() * k_star);
        // VectorXd _f = (alpha.transpose() * k_star).cwiseProduct(std) + mean;
        return _f;
    }

    // 事後分布の分散
    double var(const VectorXd &x)
    {
        VectorXd v = L.topLeftCorner(N, N).triangularView<Eigen::Lower>().solve(k_star);

        return rbf(x, x) - v.dot(v) + gaussian_variance;
    }

    MatrixXd dfdx(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        // Shape: (N, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(N, x_dim);
        for (int ix = 0; ix < N; ix++)
        {
            dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }

        return alpha.transpose() * dk_stardx;
    }

    VectorXd dsigmadx(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        // Shape: (N, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(N, x_dim);
        for (int ix = 0; ix < N; ix++)
        {
            dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }

        return -2. * k_star.transpose() * K_inv * dk_stardx;
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        VectorXd _x = x.tail(x_dim);
        // VectorXd _y = x.head(y_dim);
        VectorXd _y = x.head(y_dim);

        update_k_star(_x);

        VectorXd v = L.topLeftCorner(N, N).triangularView<Eigen::Lower>().solve(k_star);
        double sigma = rbf(x, x) - v.dot(v) + gaussian_variance;
        // double sigma = var(x);
        // VectorXd sigma = (rbf(x, x) - v.dot(v) + gaussian_variance) * std;

        // assert(sigma > 0)

        VectorXd _f = (alpha.transpose() * k_star) + mean;
        // VectorXd _f = (alpha.transpose() * k_star).cwiseProduct(st) + mean;
        // VectorXd _f = f(x);

        // yの勾配
        VectorXd y_grad = (_y - _f).cwiseProduct(st).cwiseProduct(st) / sigma;

        // p(y|x)と相似な値
        double pyx = (_y - _f).cwiseProduct(st).squaredNorm() / sigma;

        // Shape: (N, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(N, x_dim);
        for (int ix = 0; ix < N; ix++)
        {
            dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }

        // Shape: (dim_y, dim_x)
        // MatrixXd _dfdx = dfdx(x);
        MatrixXd _dfdx = alpha.transpose() * dk_stardx;

        // (dim_x)
        // VectorXd _dsigmadx = dsigmadx(x);
        VectorXd _dsigmadx = -2. * k_star.transpose() * K_inv * dk_stardx;

        // xの勾配
        x_grad.tail(x_dim) = -_dfdx.transpose() * y_grad + _dsigmadx * (y_dim - pyx) / (2 * sigma) + _x;
        x_grad.head(y_dim) = y_grad;
        // x_grad.head(y_dim) = (y_grad + mean).cwiseProduct(st);

        // 尤度
        return (pyx + (y_dim * log(sigma)) + _x.squaredNorm()) / 2;
    }
};