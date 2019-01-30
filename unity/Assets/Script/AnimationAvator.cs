﻿using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;

using Neuron;

public class AnimationAvator : MonoBehaviour
{

    private Actor actor;
    private IntPtr solver;

    private Animator animator;
    public Transform LeftHandTarget;
    public Transform RightHandTarget;
    public Transform HeadTarget;

    private string SkeletonPath;
    private string GPModelPath;

    public string SkeletonName = "skeleton.json";
    public string GPModelName = "expmap_model_reduce.json";

    public Transform Point;

    [Range(1, 300)] public int max_iterations = 20;

    public float IKWeight = 0.01f;

    private double kernel_lengthscale = 0.1;
    private double kernel_variance;
    private double gaussian_variance;

    public double KernelLengthScale
    {
        get { return kernel_lengthscale; }
    }
    public double KernelVariance{
        get{return kernel_variance; }
    }
    public double GaussianVariance{
        get{return gaussian_variance; }
    }

    public bool useExample = false;

    [Range(0, 79)] public int frame = 30;

    private Vector3 initialHipPos;


    void Awake()
    {

        SkeletonPath = Path.Combine(Application.dataPath, Path.Combine("Plugins", SkeletonName));
        GPModelPath = Path.Combine(Application.dataPath, Path.Combine("Plugins", GPModelName));

        if (File.Exists(SkeletonPath) == false || File.Exists(GPModelPath) == false)
        {
            throw new FileNotFoundException();
        }

        actor = GetComponent<Actor>();
        animator = GetComponent<Animator>();

        solver = VRIKSolver.Create(SkeletonPath, GPModelPath);
        VRIKSolver.CreateRightHandSolver(solver);
        VRIKSolver.CreateLeftHandSolver(solver);

        VRIKSolver.SetLambda(solver, IKWeight);
        VRIKSolver.SetMaxIterations(solver, max_iterations);

        initialHipPos = animator.GetBoneTransform(HumanBodyBones.Hips).position;

        kernel_lengthscale = VRIKSolver.GetKernelLengthScale(solver);
        kernel_variance = VRIKSolver.GetKernelVariance(solver);
        gaussian_variance = VRIKSolver.GetGaussianVariance(solver);

        Debug.Log(kernel_lengthscale);

    }

    void Update()
    {

        Transform t = animator.GetBoneTransform(HumanBodyBones.Hips);

        Vector3 rgoal = RightHandTarget.position;
        Vector3 lgoal = LeftHandTarget.position;

        VRIKSolver.AddRightPositionGoal(solver, rgoal.x * 100, (rgoal.y + (initialHipPos.y - t.position.y)) * 100, rgoal.z * 100);
        VRIKSolver.AddLeftPositionGoal(solver, lgoal.x * 100, (lgoal.y + (initialHipPos.y - t.position.y)) * 100, lgoal.z * 100);

        Vector2 Rthumb = OVRInput.Get(OVRInput.RawAxis2D.RThumbstick);
        Vector2 Lthumb = OVRInput.Get(OVRInput.RawAxis2D.LThumbstick);

        if (Rthumb.x != 0.0f)
        {

            IKWeight += Rthumb.x * 1e-2f;

            if (IKWeight < 0.0f)
                IKWeight = 0.0f;

            VRIKSolver.SetLambda(solver, IKWeight);

        }

        if (Lthumb.x != 0.0f)
        {

            if (OVRInput.Get(OVRInput.RawAxis1D.LIndexTrigger) > 0.3f)
            {
                kernel_variance += (double)Lthumb.x * 1e-2;

                if (kernel_variance < 0.0)
                    kernel_variance = 0.0;

                VRIKSolver.SetKernelVariance(solver, kernel_variance);
            }
            else if (OVRInput.Get(OVRInput.RawAxis1D.LHandTrigger) > 0.3f)
            {
                gaussian_variance += (double)Lthumb.x * 1e-2;

                if (gaussian_variance < 0.0)
                    gaussian_variance = 0.0;

                VRIKSolver.SetGaussianVariance(solver, gaussian_variance);
            }
            else
            {

                kernel_lengthscale += (double)Lthumb.x * 1e-2;

                if (kernel_lengthscale < 0.0)
                    kernel_lengthscale = 0.0;

                VRIKSolver.SetKernelLengthScale(solver, kernel_lengthscale);

            }

        }

        VRIKSolver.Solve(solver);

        Point.localPosition = new Vector3(VRIKSolver.GetLatentVariable(solver, 0), VRIKSolver.GetLatentVariable(solver, 1), 0);

        // hips
        SetRotation(animator, HumanBodyBones.Hips, GetReceivedRotation(SkeletonBones.Hips));

        // legs
        SetRotation(animator, HumanBodyBones.RightUpperLeg, GetReceivedRotation(SkeletonBones.RightUpLeg));
        SetRotation(animator, HumanBodyBones.RightLowerLeg, GetReceivedRotation(SkeletonBones.RightLeg));
        SetRotation(animator, HumanBodyBones.RightFoot, GetReceivedRotation(SkeletonBones.RightFoot));
        SetRotation(animator, HumanBodyBones.LeftUpperLeg, GetReceivedRotation(SkeletonBones.LeftUpLeg));
        SetRotation(animator, HumanBodyBones.LeftLowerLeg, GetReceivedRotation(SkeletonBones.LeftLeg));
        SetRotation(animator, HumanBodyBones.LeftFoot, GetReceivedRotation(SkeletonBones.LeftFoot));

        // spine
        SetRotation(animator, HumanBodyBones.Spine, GetReceivedRotation(SkeletonBones.Spine));
        SetRotation(animator, HumanBodyBones.Chest, GetReceivedRotation(SkeletonBones.Spine1) * GetReceivedRotation(SkeletonBones.Spine2) * GetReceivedRotation(SkeletonBones.Spine3));
        SetRotation(animator, HumanBodyBones.Neck, GetReceivedRotation(SkeletonBones.Neck));
        SetRotation(animator, HumanBodyBones.Head, GetReceivedRotation(SkeletonBones.Head));

        // right arm
        SetRotation(animator, HumanBodyBones.RightShoulder, GetReceivedRotation(SkeletonBones.RightShoulder));
        SetRotation(animator, HumanBodyBones.RightUpperArm, GetReceivedRotation(SkeletonBones.RightArm));
        SetRotation(animator, HumanBodyBones.RightLowerArm, GetReceivedRotation(SkeletonBones.RightForeArm));

        // right hand
        // SetRotation(animator, HumanBodyBones.RightHand, GetReceivedRotation(SkeletonBones.RightHand));
        // animator.GetBoneTransform(HumanBodyBones.RightHand).localRotation = RightHandTarget.rotation;

        // left arm
        SetRotation(animator, HumanBodyBones.LeftShoulder, GetReceivedRotation(SkeletonBones.LeftShoulder));
        SetRotation(animator, HumanBodyBones.LeftUpperArm, GetReceivedRotation(SkeletonBones.LeftArm));
        SetRotation(animator, HumanBodyBones.LeftLowerArm, GetReceivedRotation(SkeletonBones.LeftForeArm));

        // left hand 手首の回転角はコントローラから直接入れる
        // SetRotation(animator, HumanBodyBones.LeftHand, GetReceivedRotation(SkeletonBones.LeftHand));
        // animator.GetBoneTransform(HumanBodyBones.LeftHand).localRotation = LeftHandTarget.rotation;

        t.position = initialHipPos + new Vector3(0f, GetVerticalPosition(), 0f);

    }

    void SetRotation(Animator animator, HumanBodyBones bone, Quaternion rot)
    {
        Transform t = animator.GetBoneTransform(bone);
        if (t != null)
        {
            if (!float.IsNaN(rot.x) && !float.IsNaN(rot.y) && !float.IsNaN(rot.z) && !float.IsNaN(rot.w))
            {
                t.localRotation = rot;
            }
        }
    }

    Quaternion GetReceivedRotation(SkeletonBones neuronBones)
    {
        // Log Quaternion
        Vector3 axis = new Vector3(
            VRIKSolver.GetAngle(solver, (int)neuronBones + 0, 0),
            VRIKSolver.GetAngle(solver, (int)neuronBones + 0, 1),
            VRIKSolver.GetAngle(solver, (int)neuronBones + 0, 2)
        );

        if (useExample)
        {

            axis = new Vector3(
                VRIKSolver.GetExampleAngle(solver, (int)neuronBones + 0, 0, frame),
                VRIKSolver.GetExampleAngle(solver, (int)neuronBones + 0, 1, frame),
                VRIKSolver.GetExampleAngle(solver, (int)neuronBones + 0, 2, frame)
            );

            axis = new Vector3(
                VRIKSolver.GetMeanAngle(solver, (int)neuronBones + 0, 0),
                VRIKSolver.GetMeanAngle(solver, (int)neuronBones + 0, 1),
                VRIKSolver.GetMeanAngle(solver, (int)neuronBones + 0, 2)
            );
        }

        float angle = axis.magnitude;

        axis /= angle;

        return Quaternion.AngleAxis(angle * Mathf.Rad2Deg, axis);

    }

    float GetVerticalPosition()
    {
        return VRIKSolver.GetAngle(solver, (int)(SkeletonBones.NumOfBones), 0) * 0.01f;
    }

}

// スケルトンファイルから自動構成したよいかも
public enum SkeletonBones
{
    Hips = 0,
    RightUpLeg = 1,
    RightLeg = 2,
    RightFoot = 3,
    LeftUpLeg = 4,
    LeftLeg = 5,
    LeftFoot = 6,
    Spine = 7,
    Spine1 = 8,
    Spine2 = 9,
    Spine3 = 10,
    Neck = 11,
    Head = 12,
    RightShoulder = 13,
    RightArm = 14,
    RightForeArm = 15,
    RightHand = 16,
    LeftShoulder = 17,
    LeftArm = 18,
    LeftForeArm = 19,
    LeftHand = 20,
    NumOfBones = 21
}