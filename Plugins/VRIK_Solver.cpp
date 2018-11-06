#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include <cmath>

#include "VRIK_Solver.h"
#include "json11/json11.hpp"

using namespace std;
using namespace Eigen;

// L-BFGSについて
// https://github.com/yixuan/LBFGSpp/blob/master/include/LBFGS.h
// https://abicky.net/2010/06/22/114613/

// 直線探索について
// https://github.com/yixuan/LBFGSpp/blob/master/include/LBFGS/LineSearch.h
// template <typename Foo>
// int VRIKSolver::minimize(Foo &f, VectorXd &x, float &fx)
// {
//     const int n = x.size();
//     const int fpast = m_param.past;
//     // reset(n);

//     // Evaluate function and compute gradient
//     fx = f(x, m_grad);
//     Scalar xnorm = x.norm();
//     Scalar gnorm = m_grad.norm();
//     if (fpast > 0)
//         m_fx[0] = fx;

//     // Early exit if the initial x is already a minimizer
//     if (gnorm <= m_param.epsilon * std::max(xnorm, Scalar(1.0)))
//     {
//         return 1;
//     }

//     // Initial direction
//     m_drt.noalias() = -m_grad;
//     // Initial step
//     Scalar step = Scalar(1.0) / m_drt.norm();

//     int k = 1;
//     int end = 0;
//     for (;;)
//     {
//         // Save the curent x and gradient
//         m_xp.noalias() = x;
//         m_gradp.noalias() = m_grad;

//         // Line search to update x, fx and gradient
//         // LineSearch<Scalar>::Backtracking(f, fx, x, m_grad, step, m_drt, m_xp, m_param);
//         {
//             // Decreasing and increasing factors
//             const Scalar dec = 0.5;

//             // Check the value of step
//             if (step <= Scalar(0))
//                 std::invalid_argument("'step' must be positive");

//             // Save the function value at the current x
//             const Scalar fx_init = fx;
//             // Projection of gradient on the search direction
//             const Scalar dg_init = grad.dot(drt);
//             // Make sure d points to a descent direction
//             if (dg_init > 0)
//                 std::logic_error("the moving direction increases the objective function value");

//             const Scalar dg_test = param.ftol * dg_init;
//             Scalar width;

//             int iter;
//             for (iter = 0; iter < param.max_linesearch; iter++)
//             {
//                 // x_{k+1} = x_k + step * d_k
//                 x.noalias() = xp + step * drt;
//                 // Evaluate this candidate
//                 fx = f(x, grad);

//                 // Armijo の条件
//                 if (fx > fx_init + step * dg_test)
//                 {
//                     width = dec;
//                 }
//                 else
//                 {
//                     break;
//                 }

//                 if (iter >= param.max_linesearch)
//                     throw std::runtime_error("the line search routine reached the maximum number of iterations");

//                 if (step < param.min_step)
//                     throw std::runtime_error("the line search step became smaller than the minimum value allowed");

//                 if (step > param.max_step)
//                     throw std::runtime_error("the line search step became larger than the maximum value allowed");

//                 step *= width;
//             }
//         }

//         // New x norm and gradient norm
//         xnorm = x.norm();
//         gnorm = m_grad.norm();

//         // Convergence test -- gradient
//         if (gnorm <= m_param.epsilon * std::max(xnorm, Scalar(1.0)))
//         {
//             return k;
//         }
//         // Convergence test -- objective function value
//         if (fpast > 0)
//         {
//             if (k >= fpast && std::abs((m_fx[k % fpast] - fx) / fx) < m_param.delta)
//                 return k;

//             m_fx[k % fpast] = fx;
//         }
//         // Maximum number of iterations
//         if (m_param.max_iterations != 0 && k >= m_param.max_iterations)
//         {
//             return k;
//         }

//         // Update s and y
//         // s_{k+1} = x_{k+1} - x_k
//         // y_{k+1} = g_{k+1} - g_k
//         MapVec svec(&m_s(0, end), n);
//         MapVec yvec(&m_y(0, end), n);
//         svec.noalias() = x - m_xp;
//         yvec.noalias() = m_grad - m_gradp;

//         // ys = y's = 1/rho
//         // yy = y'y
//         Scalar ys = yvec.dot(svec);
//         Scalar yy = yvec.squaredNorm();
//         m_ys[end] = ys;

//         // Recursive formula to compute d = -H * g
//         m_drt.noalias() = -m_grad;
//         int bound = std::min(m_param.m, k);
//         end = (end + 1) % m_param.m;
//         int j = end;
//         for (int i = 0; i < bound; i++)
//         {
//             j = (j + m_param.m - 1) % m_param.m;
//             MapVec sj(&m_s(0, j), n);
//             MapVec yj(&m_y(0, j), n);
//             m_alpha[j] = sj.dot(m_drt) / m_ys[j];
//             m_drt.noalias() -= m_alpha[j] * yj;
//         }e

//         m_drt *= (ys / yy);

//         for (int i = 0; i < bound; i++)
//         {
//             MapVec sj(&m_s(0, j), n);
//             MapVec yj(&m_y(0, j), n);
//             Scalar beta = yj.dot(m_drt) / m_ys[j];
//             m_drt.noalias() += (m_alpha[j] - beta) * sj;
//             j = (j + 1) % m_param.m;
//         }

//         // step = 1.0 as initial guess
//         step = Scalar(1.0);
//         k++;
//     }

//     return k;
// }

// GPのパラメータを読み込む
// void VRIKSolver::Load(const char *filename)
// {

//     // GPyで生成したパラメータの読み込み。
//     ifstream ifs(filename);
//     istreambuf_iterator<char> it(ifs);
//     istreambuf_iterator<char> last;
//     string json_str(it, last);
//     string err;

//     auto json = json11::Json::parse(json_str, err);
//     auto &arrX = json["X"].array_items();
//     auto &arrY = json["Y"].array_items();

//     D = arrX.size();
//     x_dim = arrX[0].array_items().size();
//     y_dim = arrY[0].array_items().size();
//     dim = x_dim + y_dim;

//     Y.resize(D, y_dim);

//     m = 6;

//     m_x.resize(dim);
//     m_xp.resize(dim);
//     m_grad.resize(dim);
//     m_gradp.resize(dim);
//     m_drt.resize(dim);
//     m_s.resize(dim, m);
//     m_y.resize(dim, m);
//     m_ys.resize(m);
//     m_alpha.resize(m);

//     for (int i = 0; i < D; i++)
//     {
//         VectorXd *x = new VectorXd(x_dim);
//         // VectorXd *y = new VectorXd(y_dim);

//         for (int ix = 0; ix < x_dim; ix++)
//             (*x)(ix) = arrX[i][ix].number_value();

//         for (int iy = 0; iy < y_dim; iy++)
//             Y(i, iy) = arrY[i][iy].number_value();

//         inputs.push_back(x);
//         // outputs.push_back(y);
//     }

//     kernel_variance = json["kernel"]["variance"][0].number_value();
//     kernel_lengthscale = json["kernel"]["lengthscale"][0].number_value();

//     // 共分散行列（カーネル行列）の計算
//     // 対称行列なので下半分だけ求めればよい
//     L.resize(D, D);
//     for (int i = 0; i < D; i++)
//     {
//         for (int j = 0; j <= i; j++)
//         {
//             L(i, j) = rbf((*inputs[i]), (*inputs[j]));
//         }
//     }

//     // 逆行列
//     L.selfadjointView<Lower>().evalTo(K_inv);
//     K_inv = K_inv.inverse();
// }

// void VRIKSolver::update_k_star(const VectorXd &x_star)
// {
//     k_star.resize(D);
//     for (int i = 0; i < D; i++)
//     {
//         k_star(i) = rbf(x_star, (*inputs[i]));
//     }
// }

// float VRIKSolver::likelihood(const VectorXd &x, VectorXd &x_grad)
// {
//     update_k_star(x.tail(x_dim));

//     float kxx = rbf(x.tail(x_dim), x.tail(x_dim));
//     float sigma = kxx - k_star.transpose() * K_inv * k_star;
    
//     // assert(sigma > 0)

//     VectorXd mean = Y.transpose() * K_inv * k_star;

//     // yの勾配
//     x_grad.head(y_dim) = (x.head(y_dim) - mean) / sigma;

//     float pyx = (x.head(y_dim) - mean).squaredNorm() / sigma;

//     // Shape: (D, dim_x)
//     MatrixXd dk_stardx;
//     dk_stardx.resize(D, x_dim);
//     for (int ix = 0; ix < D; ix++)
//     {
//         dk_stardx.row(ix) = ((*inputs[ix]) - x.tail(x_dim)) * k_star(ix);
//     }

//     // Shape: (dim_y, dim_x)
//     MatrixXd dfdx = Y.transpose() * K_inv * dk_stardx;

//     // (dim_x)
//     VectorXd dsigmadx = (-2.f) * k_star.transpose() * K_inv * dk_stardx;

//     // xの勾配
//     x_grad.tail(x_dim) = -dfdx.transpose() * x_grad.head(y_dim) + dsigmadx * (D - pyx) / (2 * sigma) + x.tail(x_dim);

//     // 尤度
//     return (pyx + (D * log(sigma)) + x.tail(x_dim).squaredNorm()) / 2;
// }

// GPの尤度と勾配を計算
// float VRIKSolver::likelihood(const VectorXd &x, const VectorXd &y, VectorXd &m_x_grad, VectorXd &m_y_grad)
// {
//     update_k_star(x);

//     float kxx = rbf(x, x);
//     float sigma = kxx - k_star.transpose() * K_inv * k_star;
//     VectorXd mean = Y.transpose() * K_inv * k_star;

//     // yの勾配
//     m_y_grad = (y - mean) / sigma;

//     float pyx = (y - mean).squaredNorm() / sigma;

//     // Shape: (D, dim_x)
//     MatrixXd dk_stardx;
//     dk_stardx.resize(D, x_dim);
//     for (int ix = 0; ix < D; ix++)
//     {
//         dk_stardx.row(ix) = ((*inputs[ix]) - x) * k_star(ix);
//     };

//     // Shape: (dim_y, dim_x)
//     MatrixXd dfdx = Y.transpose() * K_inv * dk_stardx;

//     // (dim_x)
//     VectorXd dsigmadx = (-2.f) * k_star.transpose() * K_inv * dk_stardx;

//     // xの勾配
//     m_x_grad = -dfdx.transpose() * m_y_grad + dsigmadx * (D - pyx) / (2 * sigma) + x;

//     // 尤度
//     return (pyx + (D * log(sigma)) + x.squaredNorm()) / 2;
// }

// RBFカーネル
// float VRIKSolver::rbf(const VectorXd &x1, const VectorXd &x2)
// {
//     // return kernel_variance * exp(-0.5 * (x1 - x2).squaredNorm());
//     return exp(-0.5 * (x1 - x2).squaredNorm());
// }