#pragma once

#include <iostream>
#include <fstream>
#include "Eigen/Dense"
#include "json11/json11.hpp"

using namespace std;
using namespace Eigen;

class GP
{

  public:
    int dim;
    int x_dim;
    int y_dim;
    int D;

    double kernel_variance;
    double kernel_lengthscale;

    double gaussian_variance;

    MatrixXd L;
    MatrixXd Y;
    MatrixXd K;
    MatrixXd K_inv;

    MatrixXd alpha;
    VectorXd k_star;

    std::vector<VectorXd *> inputs;
    std::vector<VectorXd *> outputs;

    // RBFカーネルはこれで間違いないはず
    double rbf(const VectorXd &x1, const VectorXd &x2)
    {
        return kernel_variance * exp(-0.5 * (x1 - x2).squaredNorm() / (kernel_lengthscale * kernel_lengthscale));
    }

    void load(const char *filename)
    {
        // GPyで生成したパラメータの読み込み。
        ifstream ifs(filename);
        istreambuf_iterator<char> it(ifs);
        istreambuf_iterator<char> last;
        string json_str(it, last);
        string err;

        auto json = json11::Json::parse(json_str, err);
        auto &arrX = json["X"].array_items();
        auto &arrY = json["Y"].array_items();

        D = arrX.size();
        x_dim = arrX[0].array_items().size();
        y_dim = arrY[0].array_items().size();
        dim = x_dim + y_dim;

        Y.resize(D, y_dim);

        for (int i = 0; i < D; i++)
        {
            VectorXd *x = new VectorXd(x_dim);
            // VectorXd *y = new VectorXd(y_dim);

            for (int ix = 0; ix < x_dim; ix++)
                (*x)(ix) = arrX[i][ix].number_value();

            for (int iy = 0; iy < y_dim; iy++)
                Y(i, iy) = arrY[i][iy].number_value();

            inputs.push_back(x);
            // outputs.push_back(y);
        }

        kernel_variance = json["kernel"]["variance"][0].number_value();
        kernel_lengthscale = json["kernel"]["lengthscale"][0].number_value();

        gaussian_variance = json["likelihood"]["variance"][0].number_value();

        // 共分散行列（カーネル行列）の計算
        // 対称行列なので下半分だけ求めればよい

        K.resize(D, D);
        for (int i = 0; i < D; i++)
        {
            // for (int j = 0; j <= i; j++)
            // {
            //     K(i, j) = rbf((*inputs[i]), (*inputs[j]));
            // }
            for (int j = 0; j < D; j++)
            {
                K(i, j) = rbf((*inputs[i]), (*inputs[j]));
            }
        }

        MatrixXd Ky = K;
        Ky.diagonal() += (gaussian_variance + 1e-8) * VectorXd::Ones(D);

        L.resize(D, D);
        L.topLeftCorner(D, D) = Ky.topLeftCorner(D, D).selfadjointView<Eigen::Lower>().llt().matrixL();

        alpha.resize(D, y_dim);
        alpha = L.topLeftCorner(D, D).triangularView<Lower>().solve(Y);
        L.topLeftCorner(D, D).triangularView<Eigen::Lower>().adjoint().solveInPlace(alpha);
        // alpha = Ky.llt().solve(Y);

        // 逆行列
        K_inv = L * L.transpose();
        K_inv = K_inv.inverse();

        // cout << K_inv << endl;
    }

    void update_k_star(const VectorXd &x_star)
    {
        k_star.resize(D);
        for (int i = 0; i < D; i++)
        {
            k_star(i) = rbf(x_star, (*inputs[i]));
        }
    }

    VectorXd f(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        return alpha.transpose() * k_star;
    }

    double var(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        VectorXd v = L.topLeftCorner(D, D).triangularView<Eigen::Lower>().solve(k_star);

        return rbf(x, x) - v.dot(v) + gaussian_variance;
    }

    MatrixXd dfdx(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        // Shape: (D, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(D, x_dim);
        for (int ix = 0; ix < D; ix++)
        {
            dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }

        return alpha.transpose() * dk_stardx;
    }

    VectorXd dsigmadx(const VectorXd &x)
    {

        update_k_star(x.tail(x_dim));

        // Shape: (D, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(D, x_dim);
        for (int ix = 0; ix < D; ix++)
        {
            dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix) / (kernel_lengthscale * kernel_lengthscale);
        }

        return -2. * k_star.transpose() * K_inv * dk_stardx;
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        VectorXd _x = x.tail(x_dim);
        VectorXd _y = x.head(y_dim);

        update_k_star(_x);

        VectorXd v = L.topLeftCorner(D, D).triangularView<Eigen::Lower>().solve(k_star);
        double sigma =  rbf(x, x) - v.dot(v) + gaussian_variance;
        // double sigma = var(x);

        // assert(sigma > 0)

        VectorXd mean = alpha.transpose() * k_star;
        // VectorXd mean = f(x);

        // yの勾配
        // x_grad.head(y_dim) = (_y - mean) / sigma;
        VectorXd y_grad = (_y - mean) / sigma;

        double pyx = (_y - mean).squaredNorm() / sigma;

        // Shape: (D, dim_x)
        MatrixXd dk_stardx;
        dk_stardx.resize(D, x_dim);
        for (int ix = 0; ix < D; ix++)
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
        x_grad.tail(x_dim) = -_dfdx.transpose() * y_grad + _dsigmadx * (D - pyx) / (2 * sigma) + _x;
        x_grad.head(y_dim) = y_grad;

        // 尤度
        return (pyx + (D * log(sigma)) + _x.squaredNorm()) / 2;
    }
};