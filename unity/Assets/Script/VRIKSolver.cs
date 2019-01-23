using System;
using System.Runtime.InteropServices;

public static class VRIKSolver
{

    [DllImport("VRIKSolver")]
    public static extern IntPtr Create(string skeleton, string gpmodel);

    [DllImport("VRIKSolver")]
    public static extern void SetMaxIterations(IntPtr solver, int max_iterations);

    [DllImport("VRIKSolver")]
    public static extern void SetLambda(IntPtr solver, float lambda);

    [DllImport("VRIKSolver")]
    public static extern void Solve(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern void AddRightPositionGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);

    [DllImport("VRIKSolver")]
    public static extern void AddLeftPositionGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);

    [DllImport("VRIKSolver")]
    public static extern void AddRightVelocityGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);

    [DllImport("VRIKSolver")]
    public static extern void AddLeftVelocityGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);
    
    [DllImport("VRIKSolver")]
    public static extern float GetAngle(IntPtr solver, int boneIndex, int eulerIndex);
    
    [DllImport("VRIKSolver")]
    public static extern float GetExampleAngle(IntPtr solver, int boneIndex, int eulerIndex, int frame);

    [DllImport("VRIKSolver")]
    public static extern float GetLatentVariable(IntPtr solver, int index);

    [DllImport("VRIKSolver")]
    public static extern float GetLikelihood(IntPtr solver);

}