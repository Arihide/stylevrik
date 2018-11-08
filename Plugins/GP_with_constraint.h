#include "iksolver/intern/IK_Math.h"
#include "iksolver/intern/IK_QJacobian.h"
#include "iksolver/intern/IK_QTask.h"
#include "iksolver/intern/IK_QSegment.h"

#include "Eigen/Dense"
#include "json11/json11.hpp"
#include "GP.h"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>

using namespace std;

class GPConstraint
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
    double lambda;

    GP m_gp;

    json11::Json skeleton;

    GPConstraint() {}

    void Initialize(const char *filename, GP gp)
    {

        m_gp = gp;

        // load skeleton
        ifstream ifs(filename);
        istreambuf_iterator<char> it(ifs);
        istreambuf_iterator<char> last;
        string json_str(it, last);
        string err;
        skeleton = json11::Json::parse(json_str, err);

        root = CreateSegment(14); // RightArm
        tip = CreateSegment(15);  // RightForeArm

        // これやると勾配テストが通らない。。。長さが不ぞろいだとなにがまずい？->到達できない？
        root->m_translation = Vector3d(25.876, 0, 0);
        tip->m_translation = Vector3d(28.40199, 0, 0);

        tip->SetParent(root);

        m_rootmatrix.setIdentity();

        root->SetDoFId(0);
        tip->SetDoFId(3);

        root->UpdateTransform(m_rootmatrix);

        // Vector3d ggoal(48, 161, 21);

        Vector3d goal(1, 1, 1);
        Vector3d vgoal(0, 0, 0);
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

        lambda = 1;

        m_jacobian.ArmMatrices(6, 3);
    }

    IK_QSphericalSegment *CreateSegment(int id)
    {
        IK_QSphericalSegment *seg = new IK_QSphericalSegment();

        auto &rest = skeleton["bones"].array_items()[id]["mat"].array_items();

        Vector3d mstart = Vector3d::Zero();
        // Matrix3d mrest = CreateMatrix(
        //     rest[0].number_value(), rest[1].number_value(), rest[2].number_value(),
        //     rest[3].number_value(), rest[4].number_value(), rest[5].number_value(),
        //     rest[6].number_value(), rest[7].number_value(), rest[8].number_value());
        Matrix3d mrest = CreateMatrix(
            1, 0, 0, 0, 1, 0, 0, 0, 1);
        Matrix3d mbasis = CreateMatrix(
            1, 0, 0, 0, 1, 0, 0, 0, 1);

        // リンクの長さが異なると収束しない。
        double mlength = skeleton["bones"].array_items()[id]["length"].number_value();

        seg->SetTransform(mstart, mbasis, mrest, mlength);

        return seg;
    }

    void SetGlobalGoal(Vector3d &ggoal)
    {

        Affine3d mat = Affine3d::Identity();

        // mat.matrix() << 1.0, 0.0, 0.0, 20.5538,
        //     0.0, 1.0, 0.0, 160.0075,
        //     0.0, 0.0, 1.0, -1.24851,
        //     0.0, 0.0, 0.0, 1.0;

        mat.translation() << 20.5538, 160.075, -1.24851;

        task->m_goal = mat.inverse() * ggoal;
    }

    void SetVelocityGoal(Vector3d &vgoal)
    {
        // vtask->m_goal = vgoal;
    }

    bool UpdateSegment(IK_QSegment *seg, double x, double y, double z)
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

        return false;
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        double obj;

        UpdateSegment(root, x(14 * 3 + 0), x(14 * 3 + 1), x(14 * 3 + 2));
        UpdateSegment(tip, x(14 * 3 + 3), x(14 * 3 + 4), x(14 * 3 + 5));

        root->UpdateTransform(m_rootmatrix);
        // task->ComputeJacobian(m_jacobian);
        // std::list<IK_QTask *>::iterator task;
        // for (task = tasks.begin(); task != tasks.end(); task++)
        // {
        //     (*task)->ComputeJacobian(m_jacobian);
        // }
        // m_jacobian.Invert();

        task->ComputeExpMapJacobian(m_jacobian, x.segment(14 * 3, 6));

        Vector3d d_pos = task->m_goal - tip->GlobalEnd();

        x_grad = VectorXd::Zero(m_gp.dim);

        // 目的関数とその勾配
        obj = m_gp(x, x_grad);

        x_grad.segment(14 * 3, 6) += -m_jacobian.m_jacobian.transpose() * d_pos * lambda;

        obj += d_pos.squaredNorm() * 0.5 * lambda;

        return obj;
    }
};