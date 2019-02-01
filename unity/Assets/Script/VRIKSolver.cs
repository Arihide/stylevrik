using System;
using System.Runtime.InteropServices;

public static class VRIKSolver
{

    [DllImport("VRIKSolver")]
    public static extern IntPtr Create(string skeleton, string gpmodel);
    [DllImport("VRIKSolver")]
    public static extern void CreateRightHandSolver(IntPtr solver);
    [DllImport("VRIKSolver")]
    public static extern void CreateLeftHandSolver(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern void SetMaxIterations(IntPtr solver, int max_iterations);

    [DllImport("VRIKSolver")]
    public static extern void SetLambda(IntPtr solver, float lambda);

    [DllImport("VRIKSolver")]
    public static extern void SetKernelVariance(IntPtr solver, double kernel_variance);

    [DllImport("VRIKSolver")]
    public static extern void SetKernelLengthScale(IntPtr solver, double kernel_lengthscale);

    [DllImport("VRIKSolver")]
    public static extern void SetGaussianVariance(IntPtr solver, double gaussian_variance);

    [DllImport("VRIKSolver")]
    public static extern double GetKernelVariance(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern double GetKernelLengthScale(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern double GetGaussianVariance(IntPtr solver);

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
    public static extern float GetAngle(IntPtr solver, int boneIndex, int angleIndex);
    
    [DllImport("VRIKSolver")]
    public static extern float GetExampleAngle(IntPtr solver, int boneIndex, int angleIndex, int frame);

    [DllImport("VRIKSolver")]
    public static extern float GetLearnedLatentVariable(IntPtr solver, int latentIndex, int index);

    [DllImport("VRIKSolver")]
    public static extern float GetMeanAngle(IntPtr solver, int boneIndex, int angleIndex);

    [DllImport("VRIKSolver")]
    public static extern float GetLatentVariable(IntPtr solver, int index);

    [DllImport("VRIKSolver")]
    public static extern float GetLikelihood(IntPtr solver);

    [DllImport("VRIKSolver")]
    public static extern int GetNumberOfData(IntPtr solver);

}