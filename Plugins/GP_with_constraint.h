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
#include <map>

using namespace std;

class GPConstraint
{
    // skeleton.jsonからRightArmを求めたほうが良いはず
  private:
    int RightShoulder = 13;
    int RightArm = 14;
    int RightForeArm = 15;
    int RightHand = 16;
    int LeftShoulder = 17;
    int LeftArm = 18;
    int LeftForeArm = 19;
    int LeftHand = 20;

    int rmatrows = 9;

  public:
    std::list<IK_QTask *> tasks;
    IK_QJacobian m_rjacobian;
    IK_QJacobian m_ljacobian;
    IK_QSegment *rroot;
    IK_QSegment *rtip;
    IK_QSegment *lroot;
    IK_QSegment *ltip;

    IK_QPositionTask *hptask;
    IK_QPositionTask *rptask;
    IK_QPositionTask *lptask;
    IK_QVelocityTask *rvtask;
    IK_QVelocityTask *lvtask;
    std::map<int, IK_QSegment *> segment_map;
    Affine3d m_rootmatrix;
    double lambda;

    GP m_gp;

    json11::Json skeleton;

    GPConstraint() {}

    // ロードしてIKツリー構築
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

        m_rootmatrix.setIdentity();

        IK_QSegment *seg;

        rroot = CreateSegment(RightShoulder);
        seg = CreateSegment(RightArm);      // RightArm
        rtip = CreateSegment(RightForeArm); // RightForeArm
        SetSegmentTransform(rroot, RightArm);
        SetSegmentTransform(seg, RightForeArm);
        SetSegmentTransform(rtip, RightHand);
        segment_map[RightShoulder] = rroot;
        segment_map[RightArm] = seg;
        segment_map[RightForeArm] = rtip;

        // これやると勾配テストが通らない。。。長さが不ぞろいだとなにがまずい？->到達できない？
        // rroot->m_translation = Vector3d(25.876, 0, 0);
        // rtip->m_translation = Vector3d(28.40199, 0, 0);

        rtip->SetParent(seg);
        seg->SetParent(rroot);

        rroot->SetDoFId(0);
        seg->SetDoFId(3);
        rtip->SetDoFId(6);
        rroot->UpdateTransform(m_rootmatrix);

        Vector3d goal(1, 1, 1);
        Vector3d vgoal(0, 0, 0);
        // IK_QTask *ee = new IK_QPositionTask(true, rtip, goal);
        rptask = new IK_QPositionTask(true, rtip, goal);
        rvtask = new IK_QVelocityTask(true, rtip, vgoal);

        rptask->SetId(0);
        rvtask->SetId(3);

        // 指先の関節を含めたとき
        // lroot = CreateSegment(37); //LeftArm
        // ltip = CreateSegment(38);  //LeftForeArm
        // SetSegmentTransform(lroot, 38);
        // SetSegmentTransform(ltip, 39);
        // segment_map[37] = lroot;
        // segment_map[38] = ltip;

        lroot = CreateSegment(18); //LeftArm
        ltip = CreateSegment(19);  //LeftForeArm
        SetSegmentTransform(lroot, 19);
        SetSegmentTransform(ltip, 20);
        segment_map[18] = lroot;
        segment_map[19] = ltip;

        ltip->SetParent(lroot);
        lroot->SetDoFId(0);
        ltip->SetDoFId(3);
        lptask = new IK_QPositionTask(true, ltip, goal);
        lvtask = new IK_QVelocityTask(true, ltip, goal);

        lptask->SetId(0);
        lvtask->SetId(3);

        lroot->UpdateTransform(m_rootmatrix);

        // tasks.push_back(ee);

        // int primary_size = 0;
        // for (rptask = tasks.begin(); rptask != tasks.end(); rptask++)
        // {
        //     IK_QTask *qtask = *rptask;
        //     qtask->SetId(primary_size);
        //     primary_size += qtask->Size();
        // }

        lambda = 1;

        m_rjacobian.ArmMatrices(rmatrows, 3);
        m_ljacobian.ArmMatrices(6, 3);
    }

    IK_QSphericalSegment *CreateSegment(int id)
    {
        IK_QSphericalSegment *seg = new IK_QSphericalSegment();

        return seg;
    }

    void SetSegmentTransform(IK_QSegment *seg, int id)
    {

        auto &translation = skeleton[id]["translation"].array_items();

        seg->m_translation = Vector3d(
            translation[0].number_value(),
            translation[1].number_value(),
            translation[2].number_value());
    }

    void SetRightGlobalGoal(Vector3d &ggoal)
    {
        // Affine3d mat = Affine3d::Identity();
        // mat.translation() << 20.5538, 160.075, -1.24851;
        // rptask->m_goal = mat.inverse() * ggoal;

        auto &gt = skeleton[RightShoulder]["globalTranslation"].array_items();

        rptask->m_goal(0) = ggoal(0) - gt[0].number_value();
        rptask->m_goal(1) = ggoal(1) - gt[1].number_value();
        rptask->m_goal(2) = ggoal(2) - gt[2].number_value();
    }

    void SetLeftGlobalGoal(Vector3d &ggoal)
    {
        auto &gt = skeleton[LeftArm]["globalTranslation"].array_items();

        lptask->m_goal(0) = ggoal(0) - gt[0].number_value();
        lptask->m_goal(1) = ggoal(1) - gt[1].number_value();
        lptask->m_goal(2) = ggoal(2) - gt[2].number_value();
    }

    void SetRightVelocityGoal(Vector3d &vgoal)
    {
        rvtask->m_goal = vgoal;
    }

    void SetLeftVelocityGoal(Vector3d &vgoal)
    {
        lvtask->m_goal = vgoal;
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

        for (auto i = segment_map.begin(); i != segment_map.end(); ++i)
        {
            UpdateSegment(i->second, x(i->first * 3 + 0), x(i->first * 3 + 1), x(i->first * 3 + 2));
        }

        rroot->UpdateTransform(m_rootmatrix);
        lroot->UpdateTransform(m_rootmatrix);

        // std::list<IK_QTask *>::iterator task;
        // for (task = tasks.begin(); task != tasks.end(); task++)
        // {
        //     (*task)->ComputeJacobian(m_rjacobian);
        // }

        rptask->ComputeExpMapJacobian(m_rjacobian, x.segment(RightShoulder * 3, rmatrows));
        lptask->ComputeExpMapJacobian(m_ljacobian, x.segment(LeftArm * 3, 6));

        x_grad = VectorXd::Zero(m_gp.dim);

        // 目的関数とその勾配
        obj = m_gp(x, x_grad);

        x_grad.segment(RightShoulder * 3, rmatrows) += -m_rjacobian.m_jacobian.transpose() * m_rjacobian.m_beta * lambda;
        x_grad.segment(LeftArm * 3, 6) += -m_ljacobian.m_jacobian.transpose() * m_ljacobian.m_beta * lambda;

        obj += m_rjacobian.m_beta.head(3).squaredNorm() * 0.5 * lambda;
        obj += m_ljacobian.m_beta.head(3).squaredNorm() * 0.5 * lambda;

        return obj;
    }
};