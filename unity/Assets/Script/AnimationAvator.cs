using System;
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

    public int max_iterations = 20;

    public float IKWeight = 0.5f;

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

        Point.position = new Vector3(VRIKSolver.GetLatentVariable(solver, 0), VRIKSolver.GetLatentVariable(solver, 1), 0);

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
        SetRotation(animator, HumanBodyBones.RightHand, GetReceivedRotation(SkeletonBones.RightHand));
        // SetRotation(animator, HumanBodyBones.RightThumbProximal, GetReceivedRotation(SkeletonBones.RightHandThumb1));
        // SetRotation(animator, HumanBodyBones.RightThumbIntermediate, GetReceivedRotation(SkeletonBones.RightHandThumb2));
        // SetRotation(animator, HumanBodyBones.RightThumbDistal, GetReceivedRotation(SkeletonBones.RightHandThumb3));

        // SetRotation(animator, HumanBodyBones.RightIndexProximal, GetReceivedRotation(SkeletonBones.RightHandIndex1) * GetReceivedRotation(SkeletonBones.RightInHandIndex));
        // SetRotation(animator, HumanBodyBones.RightIndexIntermediate, GetReceivedRotation(SkeletonBones.RightHandIndex2));
        // SetRotation(animator, HumanBodyBones.RightIndexDistal, GetReceivedRotation(SkeletonBones.RightHandIndex3));

        // SetRotation(animator, HumanBodyBones.RightMiddleProximal, GetReceivedRotation(SkeletonBones.RightHandMiddle1) * GetReceivedRotation(SkeletonBones.RightInHandMiddle));
        // SetRotation(animator, HumanBodyBones.RightMiddleIntermediate, GetReceivedRotation(SkeletonBones.RightHandMiddle2));
        // SetRotation(animator, HumanBodyBones.RightMiddleDistal, GetReceivedRotation(SkeletonBones.RightHandMiddle3));

        // SetRotation(animator, HumanBodyBones.RightRingProximal, GetReceivedRotation(SkeletonBones.RightHandRing1) * GetReceivedRotation(SkeletonBones.RightInHandRing));
        // SetRotation(animator, HumanBodyBones.RightRingIntermediate, GetReceivedRotation(SkeletonBones.RightHandRing2));
        // SetRotation(animator, HumanBodyBones.RightRingDistal, GetReceivedRotation(SkeletonBones.RightHandRing3));

        // SetRotation(animator, HumanBodyBones.RightLittleProximal, GetReceivedRotation(SkeletonBones.RightHandPinky1) * GetReceivedRotation(SkeletonBones.RightInHandPinky));
        // SetRotation(animator, HumanBodyBones.RightLittleIntermediate, GetReceivedRotation(SkeletonBones.RightHandPinky2));
        // SetRotation(animator, HumanBodyBones.RightLittleDistal, GetReceivedRotation(SkeletonBones.RightHandPinky3));

        // left arm
        SetRotation(animator, HumanBodyBones.LeftShoulder, GetReceivedRotation(SkeletonBones.LeftShoulder));
        SetRotation(animator, HumanBodyBones.LeftUpperArm, GetReceivedRotation(SkeletonBones.LeftArm));
        SetRotation(animator, HumanBodyBones.LeftLowerArm, GetReceivedRotation(SkeletonBones.LeftForeArm));

        // left hand
        SetRotation(animator, HumanBodyBones.LeftHand, GetReceivedRotation(SkeletonBones.LeftHand));
        // SetRotation(animator, HumanBodyBones.LeftThumbProximal, GetReceivedRotation(SkeletonBones.LeftHandThumb1));
        // SetRotation(animator, HumanBodyBones.LeftThumbIntermediate, GetReceivedRotation(SkeletonBones.LeftHandThumb2));
        // SetRotation(animator, HumanBodyBones.LeftThumbDistal, GetReceivedRotation(SkeletonBones.LeftHandThumb3));

        // SetRotation(animator, HumanBodyBones.LeftIndexProximal, GetReceivedRotation(SkeletonBones.LeftHandIndex1) * GetReceivedRotation(SkeletonBones.LeftInHandIndex));
        // SetRotation(animator, HumanBodyBones.LeftIndexIntermediate, GetReceivedRotation(SkeletonBones.LeftHandIndex2));
        // SetRotation(animator, HumanBodyBones.LeftIndexDistal, GetReceivedRotation(SkeletonBones.LeftHandIndex3));

        // SetRotation(animator, HumanBodyBones.LeftMiddleProximal, GetReceivedRotation(SkeletonBones.LeftHandMiddle1) * GetReceivedRotation(SkeletonBones.LeftInHandMiddle));
        // SetRotation(animator, HumanBodyBones.LeftMiddleIntermediate, GetReceivedRotation(SkeletonBones.LeftHandMiddle2));
        // SetRotation(animator, HumanBodyBones.LeftMiddleDistal, GetReceivedRotation(SkeletonBones.LeftHandMiddle3));

        // SetRotation(animator, HumanBodyBones.LeftRingProximal, GetReceivedRotation(SkeletonBones.LeftHandRing1) * GetReceivedRotation(SkeletonBones.LeftInHandRing));
        // SetRotation(animator, HumanBodyBones.LeftRingIntermediate, GetReceivedRotation(SkeletonBones.LeftHandRing2));
        // SetRotation(animator, HumanBodyBones.LeftRingDistal, GetReceivedRotation(SkeletonBones.LeftHandRing3));

        // SetRotation(animator, HumanBodyBones.LeftLittleProximal, GetReceivedRotation(SkeletonBones.LeftHandPinky1) * GetReceivedRotation(SkeletonBones.LeftInHandPinky));
        // SetRotation(animator, HumanBodyBones.LeftLittleIntermediate, GetReceivedRotation(SkeletonBones.LeftHandPinky2));
        // SetRotation(animator, HumanBodyBones.LeftLittleDistal, GetReceivedRotation(SkeletonBones.LeftHandPinky3));

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