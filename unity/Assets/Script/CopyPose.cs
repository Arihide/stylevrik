using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CopyPose : MonoBehaviour
{

    public Animator SourceAnimator;

    private Animator animator;

    void Start()
    {
        animator = GetComponent<Animator>();
    }

    void LateUpdate()
    {

        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightUpperLeg);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightLowerLeg);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightFoot);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftUpperLeg);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftLowerLeg);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftFoot);

        // spine
        CopyRotation(SourceAnimator, animator, HumanBodyBones.Spine);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.Neck);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.Head);

        // right arm
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightShoulder);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightUpperArm);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightLowerArm);

        // right hand
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightHand);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightThumbProximal);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightThumbIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightThumbDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightIndexIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightIndexDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightMiddleIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightMiddleDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightRingIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightRingDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightLittleIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.RightLittleDistal);

        // left arm
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftShoulder);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftUpperArm);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftLowerArm);

        // left hand
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftHand);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftThumbProximal);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftThumbIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftThumbDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftIndexIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftIndexDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftMiddleIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftMiddleDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftRingIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftRingDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftLittleIntermediate);
        CopyRotation(SourceAnimator, animator, HumanBodyBones.LeftLittleDistal);

        CopyRotation(SourceAnimator, animator, HumanBodyBones.Hips);
        CopyPosition(SourceAnimator, animator, HumanBodyBones.Hips);

    }

    void CopyRotation(Animator source, Animator target, HumanBodyBones bone)
    {

        target.GetBoneTransform(bone).localRotation = source.GetBoneTransform(bone).localRotation;

    }
    void CopyPosition(Animator source, Animator target, HumanBodyBones bone)
    {

        target.GetBoneTransform(bone).localPosition = source.GetBoneTransform(bone).localPosition;

    }

}
