using System;
using System.Runtime.InteropServices;

public static class VRIKSolver
{

    [DllImport("VRIKSolver")]
    public static extern IntPtr Create();

    [DllImport("VRIKSolver")]
    public static extern void Solve(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern void AddGoal(IntPtr solver, float goal_x, float goal_y, float goal_z);
    
    [DllImport("VRIKSolver")]
    public static extern float GetEulerAngle(IntPtr solver, int boneIndex, int eulerIndex);
    
    [DllImport("VRIKSolver")]
    public static extern float GetExampleEulerAngle(IntPtr solver, int boneIndex, int eulerIndex);

}