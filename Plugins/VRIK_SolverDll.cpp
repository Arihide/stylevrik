#if defined _WIN32 || defined __CYGWIN__
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include "Eigen/Core"
#include "VRIK_Solver.h"

extern "C"
{
    EXPORT_API VRIKSolver *Create()
    {
        VRIKSolver *solver = new VRIKSolver();

        solver->param.max_iterations = 30;

        solver->gp.load("C:/Users/Takahashi/Desktop/VRAvater/Plugins/expmap_model_reduce.json");
        solver->m_x.resize(solver->gp.dim);
        solver->m_x.head(solver->gp.y_dim) = solver->gp.Y.row(6);

        solver->gpconstraint.Initialize("C:/Users/Takahashi/Desktop/VRAvater/Plugins/skeleton1.json", solver->gp);

        return solver;
    }

    EXPORT_API void SetParent(int seg, int parent)
    {
    }

    EXPORT_API void AddGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {

        Vector3d ggoal(goal_x, goal_y, goal_z);

        solver->gpconstraint.SetGlobalGoal(ggoal);

    }

    EXPORT_API void AddVelocityGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d vgoal(goal_x, goal_y, goal_z);

        solver->gpconstraint.SetVelocityGoal(vgoal);
    }

    EXPORT_API void Solve(VRIKSolver *solver)
    {
        solver->solver.minimize(solver->gpconstraint, solver->m_x, solver->m_fx);
    }

    EXPORT_API float GetEulerAngle(VRIKSolver *solver, int boneIndex, int eulerIndex)
    {
        return solver->m_x(boneIndex * 3 + eulerIndex);
    }

    EXPORT_API float GetExampleEulerAngle(VRIKSolver *solver, int boneIndex, int eulerIndex)
    {
        return solver->gp.Y(0, boneIndex * 3 + eulerIndex);
    }

    EXPORT_API float GetLikelihood(VRIKSolver *solver)
    {
        return solver->m_fx;
    }
}