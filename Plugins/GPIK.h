#include "iksolver/IK_Math.h"
#include "iksolver/IK_QJacobian.h"
#include "iksolver/IK_QTask.h"
#include "iksolver/IK_QSegment.h"

#include "Eigen/Dense"
#include "json11/json11.hpp"
#include "GP.h"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <map>

using namespace std;

class IK_QSolver
{
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    IK_QSolver() : root(NULL)
    {
    }

    int rootID;
    IK_QSegment *root;
    IK_QJacobian *jacobian;
    std::list<IK_QPositionTask *> tasks;
};

class GPIK
{
    // skeleton.jsonからRightArmを求めたほうが良いはず
  private:
    int Spine = 7;
    int Spine1 = 8;
    int Spine2 = 9;
    int Spine3 = 10;
    int Neck = 11;
    int RightShoulder = 13;
    int RightArm = 14;
    int RightForeArm = 15;
    int RightHand = 16;
    int LeftShoulder = 17;
    int LeftArm = 18;
    int LeftForeArm = 19;
    int LeftHand = 20;

    int rmatrows = 9;
    int lmatrows = 9;

  public:
    std::list<IK_QSolver *> solvers;

    IK_QJacobian m_rjacobian;
    IK_QJacobian m_ljacobian;
    IK_QSegment *rroot;
    IK_QSegment *rtip;
    IK_QSegment *lroot;
    IK_QSegment *ltip;

    IK_QSegment *rmid;
    IK_QSegment *lmid;

    IK_QPositionTask *rptask;
    IK_QPositionTask *lptask;
    IK_QVelocityTask *rvtask;
    IK_QVelocityTask *lvtask;
    std::map<int, IK_QSegment *> segment_map;
    Affine3d m_rootmatrix;

    double lambda = 1.0;

    GP m_gp;

    json11::Json skeleton;

    // std::list<Vector3d> lts;
    // std::list<Vector3d> gts;

    GPIK() {}

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

        rptask = nullptr;
        lptask = nullptr;
        rvtask = nullptr;
        lvtask = nullptr;

        segment_map.clear();
        solvers.clear();

        ifs.close();

    }

    // void CreateSkeleton(int id, float lx, float ly, float lz, float gx, float gy, float gz){

    //     Vector3d l(lx, ly, lz);
    //     Vector3d g(gx, gy, gz);

    //     lts.insert(id, l);
    //     gts.insert(id, g);

    // }

    void CreateRightSolver(){

        rroot = CreateSegment(RightShoulder);
        rmid = CreateSegment(RightArm);      // RightArm
        rtip = CreateSegment(RightForeArm); // RightForeArm

        SetSegmentTransform(rroot, RightArm);
        SetSegmentTransform(rmid, RightForeArm);
        SetSegmentTransform(rtip, RightHand);
        
        segment_map[RightShoulder] = rroot;
        segment_map[RightArm] = rmid;
        segment_map[RightForeArm] = rtip;

        rtip->SetParent(rmid);
        rmid->SetParent(rroot);

        rroot->SetDoFId(0);
        rmid->SetDoFId(3);
        rtip->SetDoFId(6);

        rroot->UpdateTransform(m_rootmatrix);

        Vector3d goal(1, 10, 1);
        Vector3d vgoal(0, 0, 0);

        rptask = new IK_QPositionTask(true, rtip, goal);
        rvtask = new IK_QVelocityTask(true, rtip, vgoal);

        rptask->SetId(0);
        rvtask->SetId(3);

        IK_QSolver *rsolver = new IK_QSolver();
        rsolver->rootID = RightShoulder;
        rsolver->root = rroot;
        rsolver->jacobian = &m_rjacobian;
        rsolver->tasks.push_back(rptask);

        solvers.push_back(rsolver);

        m_rjacobian.ArmMatrices(rmatrows, 3);

    }

    void CreateLeftSolver(){
        lroot = CreateSegment(LeftShoulder);
        lmid = CreateSegment(LeftArm);
        ltip = CreateSegment(LeftForeArm);
        SetSegmentTransform(lroot, LeftArm);
        SetSegmentTransform(lmid, LeftForeArm);
        SetSegmentTransform(ltip, LeftHand);
        segment_map[LeftShoulder] = lroot;
        segment_map[LeftArm] = lmid;
        segment_map[LeftForeArm] = ltip;

        ltip->SetParent(lmid);
        lmid->SetParent(lroot);

        lroot->SetDoFId(0);
        lmid->SetDoFId(3);
        ltip->SetDoFId(6);

        Vector3d goal(1, 10, 1);
        Vector3d vgoal(0, 0, 0);
        
        lptask = new IK_QPositionTask(true, ltip, goal);
        lvtask = new IK_QVelocityTask(true, ltip, goal);

        lptask->SetId(0);
        lvtask->SetId(3);

        lroot->UpdateTransform(m_rootmatrix);


        IK_QSolver *lsolver = new IK_QSolver();
        lsolver->rootID = LeftShoulder;
        lsolver->root = lroot;
        lsolver->jacobian = &m_ljacobian;
        lsolver->tasks.push_back(lptask);

        solvers.push_back(lsolver);

        m_ljacobian.ArmMatrices(lmatrows, 3);
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

        if(rptask == nullptr)
            return;

        auto &gt = skeleton[RightShoulder]["globalTranslation"].array_items();

        rptask->m_goal(0) = ggoal(0) - gt[0].number_value();
        rptask->m_goal(1) = ggoal(1) - gt[1].number_value();
        rptask->m_goal(2) = ggoal(2) - gt[2].number_value();
    }

    void SetLeftGlobalGoal(Vector3d &ggoal)
    {

        if(lptask == nullptr)
            return;

        auto &gt = skeleton[LeftShoulder]["globalTranslation"].array_items();

        lptask->m_goal(0) = ggoal(0) - gt[0].number_value();
        lptask->m_goal(1) = ggoal(1) - gt[1].number_value();
        lptask->m_goal(2) = ggoal(2) - gt[2].number_value();
    }

    void SetRightVelocityGoal(Vector3d &vgoal)
    {

        if(rvtask == nullptr)
            return;

        rvtask->m_goal = vgoal;
    }

    void SetLeftVelocityGoal(Vector3d &vgoal)
    {

        if(lvtask == nullptr)
            return;

        lvtask->m_goal = vgoal;
    }

    double operator()(const VectorXd &x, VectorXd &x_grad)
    {
        double obj;

        for (auto i = segment_map.begin(); i != segment_map.end(); ++i)
        {
            i->second->UpdateAngle(x.segment<3>(i->first * 3));
        }

        x_grad = VectorXd::Zero(m_gp.dim);

        obj = m_gp(x, x_grad);

        std::list<IK_QSolver *>::iterator solver;
        for (solver = solvers.begin(); solver != solvers.end(); solver++)
        {

            const int matrow = 9;

            std::list<IK_QPositionTask *>::iterator task;
            IK_QJacobian *jac = (*solver)->jacobian;

            (*solver)->root->UpdateTransform(m_rootmatrix);

            for (task = (*solver)->tasks.begin(); task != (*solver)->tasks.end(); task++)
            {
                (*task)->ComputeExpMapJacobian(*jac, x.segment((*solver)->rootID * 3, matrow));
            }

            jac->Transpose();

            // x_grad.segment((*solver)->rootID * 3, matrow) += -jac->m_jacobian.transpose() * jac->m_beta * lambda;
            x_grad.segment((*solver)->rootID * 3, matrow) += -jac->m_d_theta * lambda;

            obj += jac->m_beta.head(3).squaredNorm() * 0.5 * lambda;

        }

        return obj;
    }
};