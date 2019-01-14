using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CopyRotation : MonoBehaviour
{
    public Transform target;
    public Vector3 offsetEuler;
    private Quaternion initialRot;
    private Quaternion initialTargetRotInv;
    private Quaternion offsetRot;

    // Use this for initialization
    void Start()
    {
        offsetRot = Quaternion.Euler(offsetEuler.x, offsetEuler.y, offsetEuler.z);

        initialRot = this.transform.localRotation;
        initialTargetRotInv = Quaternion.Inverse(target.transform.localRotation);
    }

    // Update is called once per frame
    void Update()
    {

        Quaternion diffRot = target.transform.localRotation * initialTargetRotInv;
        this.transform.localRotation = offsetRot * diffRot * Quaternion.Inverse(offsetRot) * initialRot;

    }
}
