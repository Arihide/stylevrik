using System;
using UnityEngine;

public class SetParameterFromController : MonoBehaviour
{

    public AnimationAvator aniava;
    private IntPtr solver;

    private double kernel_lengthscale;
    private double kernel_variance;
    private double gaussian_variance;

    void Start()
    {

        solver = aniava.Solver;

    }

    void LateUpdate()
    {

        Vector2 Rthumb = OVRInput.Get(OVRInput.RawAxis2D.RThumbstick);
        Vector2 Lthumb = OVRInput.Get(OVRInput.RawAxis2D.LThumbstick);

        if (Rthumb.x != 0.0f)
        {

            aniava.IKWeight += Rthumb.x * 1e-2f;

            if (aniava.IKWeight < 0.0f)
                aniava.IKWeight = 0.0f;

            VRIKSolver.SetLambda(solver, aniava.IKWeight);

        }

        if (Lthumb.x != 0.0f)
        {

            if (OVRInput.Get(OVRInput.RawAxis1D.LIndexTrigger) > 0.3f)
            {
                kernel_variance = VRIKSolver.GetKernelVariance(solver);

                kernel_variance += (double)Lthumb.x * 1e-2;

                if (kernel_variance < 0.0)
                    kernel_variance = 0.0;

                VRIKSolver.SetKernelVariance(solver, kernel_variance);
            }
            else if (OVRInput.Get(OVRInput.RawAxis1D.LHandTrigger) > 0.3f)
            {
                gaussian_variance = VRIKSolver.GetGaussianVariance(solver);

                gaussian_variance += (double)Lthumb.x * 1e-2;

                if (gaussian_variance < 0.0)
                    gaussian_variance = 0.0;

                VRIKSolver.SetGaussianVariance(solver, gaussian_variance);
            }
            else
            {
                kernel_lengthscale = VRIKSolver.GetKernelLengthScale(solver);

                kernel_lengthscale += (double)Lthumb.x * 1e-2;

                if (kernel_lengthscale < 0.0)
                    kernel_lengthscale = 0.0;

                VRIKSolver.SetKernelLengthScale(solver, kernel_lengthscale);

            }

        }

    }

}