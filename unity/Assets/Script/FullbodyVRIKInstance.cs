using System.Collections.Generic;
using UnityEngine;

public class FullbodyVRIKInstance : MonoBehaviour
{
    public string address = "127.0.0.1";
    public int port = 7001;
    int commandServerPort = -1;
    // public NeuronConnection.SocketType socketType = NeuronConnection.SocketType.TCP;
    public int actorID = 0;
    public bool connectToAxis = false;
    public float hipHeightOffset = 0.035f;
        
    protected bool standalone = true;
    protected int lastEvaluateTime = 0;
    protected bool boneSizesDirty = false;
    protected bool applyBoneSizes = false;

    public bool noFrameData { get; private set; }

    private bool Connect()
    {

        return false;

    }

    void OnEnable()
    {
        DataReader.BRStartUDPServiceAt(port);
    }

    void OnDisable()
    {
        // if (boundActor != null && standalone)
        // {
            Disconnect();
        //     boundActor = null;
        // }
    }
    void Disconnect()
    {
        DataReader.BRCloseSocket();
    }

    static void SetPosition(Animator animator, HumanBodyBones bone, Vector3 pos)
    {
        Transform t = animator.GetBoneTransform(bone);
        if (t != null)
        {
            if (!float.IsNaN(pos.x) && !float.IsNaN(pos.y) && !float.IsNaN(pos.z))
            {
                t.localPosition = pos;
            }
        }
    }

    // set rotation for bone in animator
    static void SetRotation(Animator animator, HumanBodyBones bone, Vector3 rotation)
    {
        Transform t = animator.GetBoneTransform(bone);
        if (t != null)
        {
            Quaternion rot = Quaternion.Euler(rotation);
            if (!float.IsNaN(rot.x) && !float.IsNaN(rot.y) && !float.IsNaN(rot.z) && !float.IsNaN(rot.w))
            {
                t.localRotation = rot;
            }
        }
    }

}