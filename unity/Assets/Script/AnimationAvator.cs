﻿using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

using Neuron;

public class AnimationAvator : MonoBehaviour
{

    private Actor actor;
    private IntPtr solver;

    private Animator animator;
    public Transform LeftHandTarget;
    public Transform RightHandTarget;
    public Transform HeadTarget;

    void Awake()
    {

        actor = GetComponent<Actor>();
        animator = GetComponent<Animator>();

        solver = VRIKSolver.Create();

    }

    void Update()
    {
        Vector3 goal = RightHandTarget.position;
        
        VRIKSolver.AddGoal(solver, goal.x * 100, goal.y * 100, goal.z * 100);

        VRIKSolver.Solve(solver);

        // legs
        SetRotation(animator, HumanBodyBones.RightUpperLeg, GetReceivedRotation(NeuronBones.RightUpLeg));
        SetRotation(animator, HumanBodyBones.RightLowerLeg, GetReceivedRotation(NeuronBones.RightLeg));
        SetRotation(animator, HumanBodyBones.RightFoot, GetReceivedRotation(NeuronBones.RightFoot));
        SetRotation(animator, HumanBodyBones.LeftUpperLeg, GetReceivedRotation(NeuronBones.LeftUpLeg));
        SetRotation(animator, HumanBodyBones.LeftLowerLeg, GetReceivedRotation(NeuronBones.LeftLeg));
        SetRotation(animator, HumanBodyBones.LeftFoot, GetReceivedRotation(NeuronBones.LeftFoot));

        // spine
        SetRotation(animator, HumanBodyBones.Spine, GetReceivedRotation(NeuronBones.Spine));
        SetRotation(animator, HumanBodyBones.Chest, GetReceivedRotation(NeuronBones.Spine1) * GetReceivedRotation(NeuronBones.Spine2) * GetReceivedRotation(NeuronBones.Spine3));
        SetRotation(animator, HumanBodyBones.Neck, GetReceivedRotation(NeuronBones.Neck));
        SetRotation(animator, HumanBodyBones.Head, GetReceivedRotation(NeuronBones.Head));

        // right arm
        SetRotation(animator, HumanBodyBones.RightShoulder, GetReceivedRotation(NeuronBones.RightShoulder));
        SetRotation(animator, HumanBodyBones.RightUpperArm, GetReceivedRotation(NeuronBones.RightArm));
        SetRotation(animator, HumanBodyBones.RightLowerArm, GetReceivedRotation(NeuronBones.RightForeArm));

        // right hand
        SetRotation(animator, HumanBodyBones.RightHand, GetReceivedRotation(NeuronBones.RightHand));
        SetRotation(animator, HumanBodyBones.RightThumbProximal, GetReceivedRotation(NeuronBones.RightHandThumb1));
        SetRotation(animator, HumanBodyBones.RightThumbIntermediate, GetReceivedRotation(NeuronBones.RightHandThumb2));
        SetRotation(animator, HumanBodyBones.RightThumbDistal, GetReceivedRotation(NeuronBones.RightHandThumb3));

        SetRotation(animator, HumanBodyBones.RightIndexProximal, GetReceivedRotation(NeuronBones.RightHandIndex1) * GetReceivedRotation(NeuronBones.RightInHandIndex));
        SetRotation(animator, HumanBodyBones.RightIndexIntermediate, GetReceivedRotation(NeuronBones.RightHandIndex2));
        SetRotation(animator, HumanBodyBones.RightIndexDistal, GetReceivedRotation(NeuronBones.RightHandIndex3));

        SetRotation(animator, HumanBodyBones.RightMiddleProximal, GetReceivedRotation(NeuronBones.RightHandMiddle1) * GetReceivedRotation(NeuronBones.RightInHandMiddle));
        SetRotation(animator, HumanBodyBones.RightMiddleIntermediate, GetReceivedRotation(NeuronBones.RightHandMiddle2));
        SetRotation(animator, HumanBodyBones.RightMiddleDistal, GetReceivedRotation(NeuronBones.RightHandMiddle3));

        SetRotation(animator, HumanBodyBones.RightRingProximal, GetReceivedRotation(NeuronBones.RightHandRing1) * GetReceivedRotation(NeuronBones.RightInHandRing));
        SetRotation(animator, HumanBodyBones.RightRingIntermediate, GetReceivedRotation(NeuronBones.RightHandRing2));
        SetRotation(animator, HumanBodyBones.RightRingDistal, GetReceivedRotation(NeuronBones.RightHandRing3));

        SetRotation(animator, HumanBodyBones.RightLittleProximal, GetReceivedRotation(NeuronBones.RightHandPinky1) * GetReceivedRotation(NeuronBones.RightInHandPinky));
        SetRotation(animator, HumanBodyBones.RightLittleIntermediate, GetReceivedRotation(NeuronBones.RightHandPinky2));
        SetRotation(animator, HumanBodyBones.RightLittleDistal, GetReceivedRotation(NeuronBones.RightHandPinky3));

        // left arm
        SetRotation(animator, HumanBodyBones.LeftShoulder, GetReceivedRotation(NeuronBones.LeftShoulder));
        SetRotation(animator, HumanBodyBones.LeftUpperArm, GetReceivedRotation(NeuronBones.LeftArm));
        SetRotation(animator, HumanBodyBones.LeftLowerArm, GetReceivedRotation(NeuronBones.LeftForeArm));

        // left hand
        SetRotation(animator, HumanBodyBones.LeftHand, GetReceivedRotation(NeuronBones.LeftHand));
        SetRotation(animator, HumanBodyBones.LeftThumbProximal, GetReceivedRotation(NeuronBones.LeftHandThumb1));
        SetRotation(animator, HumanBodyBones.LeftThumbIntermediate, GetReceivedRotation(NeuronBones.LeftHandThumb2));
        SetRotation(animator, HumanBodyBones.LeftThumbDistal, GetReceivedRotation(NeuronBones.LeftHandThumb3));

        SetRotation(animator, HumanBodyBones.LeftIndexProximal, GetReceivedRotation(NeuronBones.LeftHandIndex1) * GetReceivedRotation(NeuronBones.LeftInHandIndex));
        SetRotation(animator, HumanBodyBones.LeftIndexIntermediate, GetReceivedRotation(NeuronBones.LeftHandIndex2));
        SetRotation(animator, HumanBodyBones.LeftIndexDistal, GetReceivedRotation(NeuronBones.LeftHandIndex3));

        SetRotation(animator, HumanBodyBones.LeftMiddleProximal, GetReceivedRotation(NeuronBones.LeftHandMiddle1) * GetReceivedRotation(NeuronBones.LeftInHandMiddle));
        SetRotation(animator, HumanBodyBones.LeftMiddleIntermediate, GetReceivedRotation(NeuronBones.LeftHandMiddle2));
        SetRotation(animator, HumanBodyBones.LeftMiddleDistal, GetReceivedRotation(NeuronBones.LeftHandMiddle3));

        SetRotation(animator, HumanBodyBones.LeftRingProximal, GetReceivedRotation(NeuronBones.LeftHandRing1) * GetReceivedRotation(NeuronBones.LeftInHandRing));
        SetRotation(animator, HumanBodyBones.LeftRingIntermediate, GetReceivedRotation(NeuronBones.LeftHandRing2));
        SetRotation(animator, HumanBodyBones.LeftRingDistal, GetReceivedRotation(NeuronBones.LeftHandRing3));

        SetRotation(animator, HumanBodyBones.LeftLittleProximal, GetReceivedRotation(NeuronBones.LeftHandPinky1) * GetReceivedRotation(NeuronBones.LeftInHandPinky));
        SetRotation(animator, HumanBodyBones.LeftLittleIntermediate, GetReceivedRotation(NeuronBones.LeftHandPinky2));
        SetRotation(animator, HumanBodyBones.LeftLittleDistal, GetReceivedRotation(NeuronBones.LeftHandPinky3));

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

    // Quaternion GetReceivedRotation(NeuronBones neuronBones)
    // {

    //     return Quaternion.Euler(new Vector3(
    //         VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 1, 1),
    //         -VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 1, 0),
    //         -VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 1, 2)
    //     ));

    // }

    Quaternion GetReceivedRotation(NeuronBones neuronBones)
    {
        Vector3 axis = new Vector3(
            VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 0, 0),
            VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 0, 1),
            VRIKSolver.GetEulerAngle(solver, (int)neuronBones + 0, 2)
        );

        float angle = axis.magnitude;

        axis /= angle;

        return Quaternion.AngleAxis(angle * Mathf.Rad2Deg, axis);

    }

}
