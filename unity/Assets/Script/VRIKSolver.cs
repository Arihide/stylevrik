using System;
using System.Runtime.InteropServices;

public static class VRIKSolver
{

    [DllImport("VRIKSolver")]
    public static extern IntPtr Create();

    [DllImport("VRIKSolver")]
    public static extern void SetMaxIterations(IntPtr solver, int max_iterations);

    [DllImport("VRIKSolver")]
    public static extern void Solve(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern void AddRightPositionGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);

    [DllImport("VRIKSolver")]
    public static extern void AddLeftPositionGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);

    [DllImport("VRIKSolver")]
    public static extern void AddVelocityGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);
    
    [DllImport("VRIKSolver")]
    public static extern float GetAngle(IntPtr solver, int boneIndex, int eulerIndex);
    
    [DllImport("VRIKSolver")]
    public static extern float GetExampleAngle(IntPtr solver, int boneIndex, int eulerIndex);

    [DllImport("VRIKSolver")]
    public static extern float GetLatentVariable(IntPtr solver, int index);

    [DllImport("VRIKSolver")]
    public static extern float GetLikelihood(IntPtr solver);

}