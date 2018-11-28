#if defined _WIN32 || defined __CYGWIN__
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

#include "Eigen/Core"
#include "VRIK_Solver.h"

extern "C"
{
    EXPORT_API VRIKSolver *Create(const char *skeleton, const char *gpmodel)
    {
        VRIKSolver *solver = new VRIKSolver();

        solver->param.max_iterations = 30;

        // solver->gp.load("C:/Users/Takahashi/Desktop/VRAvater/Plugins/expmap_model_reduce.json");
        // solver->gp.load("C:/Users/Takahashi/Desktop/VRAvater/Plugins/walk01model.json");
        solver->gp.load(gpmodel);
        solver->m_x.resize(solver->gp.dim);
        solver->m_x.head(solver->gp.y_dim) = solver->gp.Y.row(6);

        // solver->gpconstraint.Initialize("C:/Users/Takahashi/Desktop/VRAvater/Plugins/skeleton.json", solver->gp);
        solver->gpconstraint.Initialize(skeleton, solver->gp);

        return solver;
    }

    EXPORT_API void SetMaxIterations(VRIKSolver *solver, int max_iterations)
    {
        solver->param.max_iterations = max_iterations;
    }

    EXPORT_API void SetLambda(VRIKSolver *solver, float lambda)
    {
        solver->gpconstraint.lambda = lambda;
    }

    EXPORT_API void AddRightPositionGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d ggoal(goal_x, goal_y, goal_z);
        solver->gpconstraint.SetRightGlobalGoal(ggoal);
    }

    EXPORT_API void AddLeftPositionGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d ggoal(goal_x, goal_y, goal_z);
        solver->gpconstraint.SetLeftGlobalGoal(ggoal);
    }

    EXPORT_API void AddRightVelocityGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d vgoal(goal_x, goal_y, goal_z);
        solver->gpconstraint.SetRightVelocityGoal(vgoal);
    }

    EXPORT_API void AddLeftVelocityGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d vgoal(goal_x, goal_y, goal_z);
        solver->gpconstraint.SetLeftVelocityGoal(vgoal);
    }

    EXPORT_API void Solve(VRIKSolver *solver)
    {
        solver->solver.minimize(solver->gpconstraint, solver->m_x, solver->m_fx);
    }

    EXPORT_API float GetAngle(VRIKSolver *solver, int boneIndex, int eulerIndex)
    {
        return solver->m_x(boneIndex * 3 + eulerIndex);
    }

    EXPORT_API float GetExampleAngle(VRIKSolver *solver, int boneIndex, int eulerIndex)
    {
        return solver->gp.Y(0, boneIndex * 3 + eulerIndex);
    }

    EXPORT_API float GetLatentVariable(VRIKSolver *solver, int index)
    {
        return solver->m_x(solver->gp.y_dim + index);
    }

    EXPORT_API float GetLikelihood(VRIKSolver *solver)
    {
        return solver->m_fx;
    }
}