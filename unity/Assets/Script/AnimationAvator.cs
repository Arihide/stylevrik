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

    [Range(1, 100)]public int max_iterations = 20;

    [Range(0, 10)]public float IKWeight = 0.5f;

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

        VRIKSolver.SetLambda(solver, IKWeight);

        VRIKSolver.SetMaxIterations(solver, max_iterations);

    }

    void Update()
    {
        Vector3 rgoal = RightHandTarget.position;
        Vector3 lgoal = LeftHandTarget.position;

        VRIKSolver.AddRightPositionGoal(solver, rgoal.x * 100, rgoal.y * 100, rgoal.z * 100);
        VRIKSolver.AddLeftPositionGoal(solver, lgoal.x * 100, lgoal.y * 100, lgoal.z * 100);

        VRIKSolver.Solve(solver);

        Point.localPosition = new Vector3(VRIKSolver.GetLatentVariable(solver, 0), VRIKSolver.GetLatentVariable(solver, 1), 0);

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

    }

    void SetRotation(Animator animator, HumanBodyBones bone, Quaternion rotation)
    {
        Transform t = animator.GetBoneTransform(bone);
        if (t != null)
        {
            // Quaternion rot = Quaternion.Euler(rotation);
            // if (!float.IsNaN(rot.x) && !float.IsNaN(rot.y) && !float.IsNaN(rot.z) && !float.IsNaN(rot.w))
            // {
            //     t.localRotation = rot;
            // }
            t.localRotation = rotation;
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

        float angle = axis.magnitude;

        axis /= angle;

        return Quaternion.AngleAxis(angle * Mathf.Rad2Deg, axis);

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
    NumOfBones
}