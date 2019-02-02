using UnityEngine;

public class SetLatentVariableFromController : MonoBehaviour
{

    public Transform Point;

    void Start()
    {



    }

    void Update()
    {

        Vector2 Lthumb = OVRInput.Get(OVRInput.RawAxis2D.LThumbstick);

        if (Lthumb.x != 0.0f && Lthumb.y != 0.0f)
        {
            Point.localPosition += new Vector3(Lthumb.x, Lthumb.y, 0f) * 0.01f;
        }

        if (OVRInput.Get(OVRInput.RawAxis1D.LIndexTrigger) > 0.1f)
        {
            Point.localPosition += new Vector3(0f, 0f, OVRInput.Get(OVRInput.RawAxis1D.LIndexTrigger)) * 0.01f;
        }
        else if (OVRInput.Get(OVRInput.RawAxis1D.LHandTrigger) > 0.1f)
        {
            Point.localPosition -= new Vector3(0f, 0f, OVRInput.Get(OVRInput.RawAxis1D.LHandTrigger) ) * 0.01f;
        }

        if(OVRInput.GetDown(OVRInput.RawButton.LThumbstick)){

            Point.localPosition = Vector3.zero;
        }

        if (OVRInput.GetDown(OVRInput.RawButton.X))
        {

            this.GetComponent<LatentPosition>().poseFromLatent = !this.GetComponent<LatentPosition>().poseFromLatent;

        }

    }



}