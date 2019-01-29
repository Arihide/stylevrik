#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "GP.h"
#include "GPIK.h"
#include "LBFGS/LBFGS.h"
#include "Eigen/Dense"

using namespace std;
using namespace Eigen;
using namespace LBFGSpp;

// 下の check_gradient 関数用の単純な関数
double func(const VectorXd &x, VectorXd &grad)
{
    VectorXd c(2);
    c << 10, 30;
    grad = 2 * (x - c) / 10;

    // ||x - c||^2
    return (x - c).squaredNorm() / 10;
}

double zero(const VectorXd &x, VectorXd &grad)
{

    grad.setZero(grad.size());

    return 0;
}

// 勾配の導関数が正しいかどうかを差分計算で確かめる関数
template <typename Foo>
bool check_gradient(Foo &f, VectorXd &x, VectorXd &x_grad)
{
    bool isGradOkay = true;
    double tolerance = 1e-3;

    VectorXd dx(x.size());

    double f1;
    double f2;
    double denominator, global_ratio, global_diff;

    double step = 1e-6;

    for (int i = 0; i < 10; i++)
    {
        x.setRandom(x.size());
        dx = step * VectorXd::Random(x.size());

        f1 = f(x + dx, x_grad);
        f2 = f(x - dx, x_grad);
        f(x, x_grad);

        denominator = 2.0 * dx.dot(x_grad);
        denominator = denominator == 0 ? 1e-24 : denominator;
        global_ratio = (f1 - f2) / denominator;
        global_diff = abs(f1 - f2) < tolerance && x_grad.norm() < 1e-1;
        // if (global_ration == NAN)
        //     global_ratio = 0;

        isGradOkay &= (abs(1.0 - global_ratio) < tolerance || global_diff);

    }

    return isGradOkay;
}

// VRIK_Solverが正しいかどうかを検証するテスト
int main()
{
    // VRIKSolver p;
    LBFGSParam<double> param;
    // param.max_iterations = 100;
    LBFGSSolver<double> solver(param);

    double fx = 0;
    VectorXd x = VectorXd::Zero(2);
    VectorXd x_grad = VectorXd::Zero(2);

    // "勾配のテスト"のテスト
    assert(check_gradient(func, x, x_grad));
    // solver.minimize(func, x, fx);

    // assert(abs(x(0) - 10) < 1e-3);
    // assert(abs(x(1) - 30) < 1e-3);
    // assert(abs(fx) < 1e-3);

    GP gp;

    // GPのモデル読み込み
    gp.load("testmodels/testmodel_reduced.json");

    x = VectorXd::Zero(gp.dim);
    x_grad = VectorXd::Zero(gp.dim);

    // 勾配の導関数が正しいか？
    assert(check_gradient(gp, x, x_grad));
    // solver.minimize(gp, x, fx);

    gp.load("testmodels/walk00_rmfinger_model.json");
    x.resize(gp.dim);
    x_grad.resize(gp.dim);
    assert(check_gradient(gp, x, x_grad));

    gp.load("testmodels/Scene_10_rmfinger_reduced_model_added.json");
    x.resize(gp.dim);
    x_grad.resize(gp.dim);
    assert(check_gradient(gp, x, x_grad));
    solver.minimize(gp, x, fx);
    

    // なぜか最小化できない

    // IK
    GPIK gpik;
    gpik.Initialize("testmodels/skeleton_reduced.json", gp);
    // gpik.CreateRightSolver();
    gpik.CreateLeftSolver();
    x = VectorXd::Zero(gp.dim);
    x_grad = VectorXd::Zero(gp.dim);
    assert(check_gradient(gpik, x, x_grad));
    solver.minimize(gpik, x, fx);

    cout << "All tests were passed" << endl;

    return 0;
}