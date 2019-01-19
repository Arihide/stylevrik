#if defined _WIN32 || defined __CYGWIN__
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

// #include "stdafx.h" //Use when compiling from Visual Studio
#include <iostream>
#include "iksolver/extern/IK_solver.h"

extern "C"
{

    EXPORT_API IK_Segment *CreateSegment(int flag)
    {
        return IK_CreateSegment(flag);
    }

    EXPORT_API void FreeSegment(IK_Segment *seg)
    {
        IK_FreeSegment(seg);
    }

    EXPORT_API void SetParent(IK_Segment *seg, IK_Segment *parent)
    {
        IK_SetParent(seg, parent);
    }

    EXPORT_API void SetNullParent(IK_Segment *seg)
    {
        IK_SetParent(seg, NULL);
    }

    // Unityには4x4行列しかないので、cpp側がそれに合わせたほうが良い？
    EXPORT_API void SetTransform(IK_Segment *seg, float *start, float rest_basis[][3], float basis[][3], float length)
    {
        IK_SetTransform(seg, start, rest_basis, basis, length);
    }

    EXPORT_API IK_Solver *CreateSolver(IK_Segment *root)
    {
        return IK_CreateSolver(root);
    }

    EXPORT_API void FreeSolver(IK_Solver *solver)
    {
        IK_FreeSolver(solver);
    }

    EXPORT_API void SolverAddGoal(IK_Solver *solver, IK_Segment *tip, float *goal, float weight)
    {
        IK_SolverAddGoal(solver, tip, goal, weight);
    }

    EXPORT_API void Solve(IK_Solver *solver, float tolerance, int max_iterations)
    {
        IK_Solve(solver, tolerance, max_iterations);
    }

    EXPORT_API void GetBasisChange(IK_Segment *seg, float basis_change[][3])
    {
        IK_GetBasisChange(seg, basis_change);
    }

    // EXPORT_API void CreateTestIKChain()
    // {

    //     IK_Segment *seg, *parent, **iktree, *iktarget;
    //     IK_Solver *solver;

    //     iktree = MEM_mallocN(sizeof(void *) * 3, "ik_tree");

    //     for (int a = 0; a < 3; a++)
    //     {
    //         seg = iktree[a] = IK_CreateSegment(0);

    //         if (a == 0)
    //             parent = NULL;
    //         else
    //             parent = seg;

    //         IK_SetParent(seg, parent);

    //         IK_SetTransform(seg)
    //     }

    //     solver = IK_CreateSolver(iktree[0]);

    //     iktarget = iktree[3] IK_SolverAddGoal(solver, iktarget);

    //     IK_Solve(solver, 0.0f, 100);
    // }
}