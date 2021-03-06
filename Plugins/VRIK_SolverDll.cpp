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

        solver->gp.load(gpmodel);
        solver->m_x.resize(solver->gp.dim);
        solver->m_x.head(solver->gp.y_dim) = solver->gp.mean;

        solver->gpik.Initialize(skeleton, solver->gp);

        return solver;
    }

    EXPORT_API void CreateRightHandSolver(VRIKSolver *solver)
    {
        solver->gpik.CreateRightSolver();
    }

    EXPORT_API void CreateLeftHandSolver(VRIKSolver *solver)
    {
        solver->gpik.CreateLeftSolver();
    }

    EXPORT_API void SetMaxIterations(VRIKSolver *solver, int max_iterations)
    {
        solver->param.max_iterations = max_iterations;
    }

    EXPORT_API void SetLambda(VRIKSolver *solver, float lambda)
    {
        solver->gpik.lambda = lambda;
    }

    EXPORT_API void SetKernelVariance(VRIKSolver *solver, double kernel_variance)
    {
        solver->gp.kernel_variance = kernel_variance;
    }

    EXPORT_API void SetKernelLengthScale(VRIKSolver *solver, double kernel_lengthscale)
    {
        solver->gp.kernel_lengthscale = kernel_lengthscale;
    }

    EXPORT_API void SetGaussianVariance(VRIKSolver *solver, double gaussian_variance)
    {
        solver->gp.gaussian_variance = gaussian_variance;
    }

    EXPORT_API void SetJoint(VRIKSolver *solver)
    {
    }

    EXPORT_API double GetKernelVariance(VRIKSolver *solver)
    {
        return solver->gp.kernel_variance;
    }

    EXPORT_API double GetKernelLengthScale(VRIKSolver *solver)
    {
        return solver->gp.kernel_lengthscale;
    }

    EXPORT_API double GetGaussianVariance(VRIKSolver *solver)
    {
        return solver->gp.gaussian_variance;
    }

    EXPORT_API void AddHeadPositionGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d ggoal(goal_x, goal_y, goal_z);
    }

    EXPORT_API void AddRightPositionGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d ggoal(goal_x, goal_y, goal_z);
        solver->gpik.SetRightGlobalGoal(ggoal);
    }

    EXPORT_API void AddLeftPositionGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d ggoal(goal_x, goal_y, goal_z);
        solver->gpik.SetLeftGlobalGoal(ggoal);
    }

    EXPORT_API void AddRightVelocityGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d vgoal(goal_x, goal_y, goal_z);
        solver->gpik.SetRightVelocityGoal(vgoal);
    }

    EXPORT_API void AddLeftVelocityGoal(VRIKSolver *solver, float goal_x, float goal_y, float goal_z)
    {
        Vector3d vgoal(goal_x, goal_y, goal_z);
        solver->gpik.SetLeftVelocityGoal(vgoal);
    }

    EXPORT_API void Solve(VRIKSolver *solver)
    {
        solver->solver.minimize(solver->gpik, solver->m_x, solver->m_fx);
    }

    EXPORT_API int GetNumberOfData(VRIKSolver *solver)
    {
        return solver->gp.N;
    }

    EXPORT_API float GetAngle(VRIKSolver *solver, int boneIndex, int angleIndex)
    {
        return solver->m_x(boneIndex * 3 + angleIndex);
    }

    EXPORT_API float GetExampleAngle(VRIKSolver *solver, int boneIndex, int angleIndex, int frame)
    {
        return solver->gp.Y(frame, boneIndex * 3 + angleIndex);
    }

    EXPORT_API float GetLearnedLatentVariable(VRIKSolver *solver, int latentIndex, int index)
    {
        return (*solver->gp.inputs[latentIndex])(index);
    }

    EXPORT_API float GetMeanAngle(VRIKSolver *solver, int boneIndex, int angleIndex)
    {
        return solver->gp.mean(boneIndex * 3 + angleIndex);
    }

    EXPORT_API float GetLatentVariable(VRIKSolver *solver, int index)
    {
        return solver->m_x(solver->gp.y_dim + index);
    }

    EXPORT_API float GetLikelihood(VRIKSolver *solver)
    {
        return solver->m_fx;
    }

    EXPORT_API void Predict(VRIKSolver *solver, float x, float y, float z)
    {
        Vector3d x_star(x, y, z);

        solver->m_x.tail(solver->gp.x_dim) = x_star;

        solver->gp.update_k_star(x_star);
        solver->m_x.head(solver->gp.y_dim) = solver->gp.f();

    }
}