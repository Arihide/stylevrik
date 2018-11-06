#include "iksolver/intern/IK_Math.h"
#include "iksolver/intern/IK_QJacobian.h"
#include "iksolver/intern/IK_QTask.h"
#include "iksolver/intern/IK_QSegment.h"

#include "Eigen/Dense"
#include "json11/json11.hpp"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>

using namespace std;

class IK
{
  public:
    std::list<IK_QTask *> tasks;
    IK_QJacobian m_jacobian;
    IK_QSegment *root;
    IK_QSegment *tip;
    IK_QPositionTask *task;
    IK_QVelocityTask *vtask;
    std::vector<IK_QSegment *> m_segment;
    Affine3d m_rootmatrix;
    Vector3d goal;

    json11::Json skeleton;

    IK()
    {
        root = new IK_QSphericalSegment();
        tip = new IK_QSphericalSegment();

        tip->SetParent(root);

        Vector3d mstart(0, 0, 0);
        Matrix3d mrest = CreateMatrix(
            1, 0, 0, 0, 1, 0, 0, 0, 1);
        Matrix3d mbasis = CreateMatrix(
            1, 0, 0, 0, 1, 0, 0, 0, 1);
        double mlength(10);

        root->SetTransform(mstart, mrest, mbasis, mlength);
        tip->SetTransform(mstart, mrest, mbasis, mlength);

        m_rootmatrix.setIdentity();

        root->SetDoFId(0);
        tip->SetDoFId(3);

        root->UpdateTransform(m_rootmatrix);

        goal << 1, 10, 1;

        Vector3d vgoal;
        vgoal << 0.5, 0.2, 0.2;

        task = new IK_QPositionTask(true, tip, goal);
        vtask = new IK_QVelocityTask(true, tip, vgoal);

        task->SetId(0);
        vtask->SetId(3);
        // tasks.push_back(ee);

        // int primary_size = 0;
        // for (task = tasks.begin(); task != tasks.end(); task++)
        // {
        //     IK_QTask *qtask = *task;
        //     qtask->SetId(primary_size);
        //     primary_size += qtask->Size();
        // }

        // m_jacobian.ArmMatrices(6, 3);
        m_jacobian.ArmMatrices(9, 6);
    }

    IK(const char *filename)
    {

        // load skeleton
        ifstream ifs(filename);
        istreambuf_iterator<char> it(ifs);
        istreambuf_iterator<char> last;
        string json_str(it, last);
        string err;
        skeleton = json11::Json::parse(json_str, err);

        root = CreateSegment(14); // RightArm
        tip = CreateSegment(15);  // RightForeArm
        tip->SetParent(root);

        m_rootmatrix.setIdentity();

        root->SetDoFId(0);
        tip->SetDoFId(3);

        root->UpdateTransform(m_rootmatrix);

        goal << 1, 1, 1;
        Vector3d vgoal;
        vgoal << 1, 1, 1;

        // IK_QTask *ee = new IK_QPositionTask(true, tip, goal);
        task = new IK_QPositionTask(true, tip, goal);
        vtask = new IK_QVelocityTask(true, tip, vgoal);

        task->SetId(0);
        vtask->SetId(3);
        // tasks.push_back(ee);

        // int primary_size = 0;
        // for (task = tasks.begin(); task != tasks.end(); task++)
        // {
        //     IK_QTask *qtask = *task;
        //     qtask->SetId(primary_size);
        //     primary_size += qtask->Size();
        // }

        // (変数, エフェクタ)
        // m_jacobian.ArmMatrices(6, 3);
        m_jacobian.ArmMatrices(9, 6);
    }

    IK_QSphericalSegment *CreateSegment(int id)
    {
        int flag = 7;
        // IK_QSegment *seg = CreateSegment(flag, false);
        IK_QSphericalSegment *seg = new IK_QSphericalSegment();

        auto &rest = skeleton["bones"].array_items()[id]["mat"].array_items();

        Vector3d mstart = Vector3d::Zero();
        Matrix3d mrest = CreateMatrix(
            rest[0].number_value(), rest[1].number_value(), rest[2].number_value(),
            rest[3].number_value(), rest[4].number_value(), rest[5].number_value(),
            rest[6].number_value(), rest[7].number_value(), rest[8].number_value());
        Matrix3d mbasis = CreateMatrix(
            1, 0, 0, 0, 1, 0, 0, 0, 1);

        // double mlength = skeleton["bones"].array_items()[id]["length"].number_value();
        double mlength(10);

        cout << mrest << endl;

        seg->SetTransform(mstart, mbasis, mrest, mlength);

        return seg;
    }

    void SetGlobalGoal(Vector3d &ggoal)
    {

        Affine3d mat;
        mat.matrix() << 0, -1, 0, -17.5, 1, 0, -1.62921e-07, 164.969, 8.74225e-08, 0, 1, 2.68769e-05, 0, 0, 0, 1;

        goal = mat.inverse() * ggoal;

        cout << goal << endl;
    }

    void UpdateSegment(IK_QSegment *seg, double x, double y, double z)
    {
        Vector3d q(x, y, z);

        // Directly update the rotation matrix, with Rodrigues' rotation formula,
        // to avoid singularities and allow smooth integration.

        double theta = q.norm();

        if (!FuzzyZero(theta))
        {
            Vector3d w = q * (1.0 / theta);

            double sine = sin(theta);
            double cosine = cos(theta);
            double cosineInv = 1 - cosine;

            double xsine = w.x() * sine;
            double ysine = w.y() * sine;
            double zsine = w.z() * sine;

            double xxcosine = w.x() * w.x() * cosineInv;
            double xycosine = w.x() * w.y() * cosineInv;
            double xzcosine = w.x() * w.z() * cosineInv;
            double yycosine = w.y() * w.y() * cosineInv;
            double yzcosine = w.y() * w.z() * cosineInv;
            double zzcosine = w.z() * w.z() * cosineInv;

            Matrix3d M = CreateMatrix(
                cosine + xxcosine, -zsine + xycosine, ysine + xzcosine,
                zsine + xycosine, cosine + yycosine, -xsine + yzcosine,
                -ysine + xzcosine, xsine + yzcosine, cosine + zzcosine);

            seg->m_basis = M;
        }
        else
        {
            seg->m_basis = CreateMatrix(
                1, 0, 0, 0, 1, 0, 0, 0, 1);
        }
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        double obj = 0;

        UpdateSegment(root, x(0), x(1), x(2));
        UpdateSegment(tip, x(3), x(4), x(5));

        root->UpdateTransform(m_rootmatrix);

        root->SetDoFId(0);
        tip->SetDoFId(3);
        task->ComputeExpMapJacobian(m_jacobian, x);
        // root->SetDoFId(6);
        // tip->SetDoFId(9);
        vtask->ComputeExpMapJacobian(m_jacobian, x.tail(3));

        x_grad = -m_jacobian.m_jacobian.transpose() * m_jacobian.m_beta;

        // x_grad.tail(6) = VectorXd::Zero(6);

        obj += m_jacobian.m_beta.head(3).squaredNorm() * 0.5;
        obj += m_jacobian.m_beta.tail(3).squaredNorm() * 0.5;

        cout << m_jacobian.m_jacobian << endl;
        // cout << m_jacobian.m_beta << endl;

        return obj;
    }
};