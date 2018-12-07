#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "GP.h"
#include "GP_with_constraint.h"
#include "ik_experiment.h"
#include "LBFGS.h"
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

        // cout << isGradOkay << endl;

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
    // assert(check_gradient(func, x, x_grad));
    solver.minimize(func, x, fx);

    assert(abs(x(0) - 10) < 1e-3);
    assert(abs(x(1) - 30) < 1e-3);
    assert(abs(fx) < 1e-3);

    // GPパラメータの読み込み
    GP gp;
    // gp.load("testmodel.json");

    // 読み込んだパラメータのテスト
    // GPの尤度の勾配のテスト
    // x = VectorXd::Ones(gp.dim);
    // x_grad = VectorXd::Ones(gp.dim);
    // assert(check_gradient(gp, x, x_grad));
    // solver.minimize(gp, x, fx);
    // cout << x << endl;

    // 別のモデルでもテスト
    // gp.load("expmap_model_reduce.json");
    // gp.load("testmodel.json");
    gp.load("testmodel_3dim.json");
    // gp.load("testmodel.1.json");
    // gp.load("testmodel_with_vel.json");
    x = VectorXd::Zero(gp.dim);
    x_grad = VectorXd::Zero(gp.dim);
    // 勾配の導関数が正しいか？
    assert(check_gradient(gp, x, x_grad));
    solver.minimize(gp, x, fx);

    // IKのテスト
    // IK ik("skeleton.json");
    IK ik;
    x.setZero(9);
    x_grad.setZero(9);
    assert(check_gradient(ik, x, x_grad));
    // solver.minimize(ik, x, fx);

    // IK
    GPConstraint gpcontraint;
    // gpcontraint.Initialize("skeleton.json", gp);
    gpcontraint.Initialize("skeleton_reduced.json", gp);
    x = VectorXd::Zero(gp.dim);
    x_grad = VectorXd::Zero(gp.dim);
    // assert(check_gradient(gpcontraint, x, x_grad));
    // solver.minimize(gpcontraint, x, fx);

    // gpcontraint(x, x_grad);
    // assert(check_gradient(gpconstraint, x, x_grad));

    cout << "All tests were passed" << endl;

    return 0;
}