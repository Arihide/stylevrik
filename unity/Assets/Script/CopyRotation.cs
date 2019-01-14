using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CopyRotation : MonoBehaviour
{
    public Transform target;
    public Vector3 initialEuler;
    public Vector3 offsetEuler;
    private Quaternion initialRot;
    private Quaternion initialTargetRotInv;
    private Quaternion offsetRot;

    // Use this for initialization
    void Start()
    {
        offsetRot = Quaternion.Euler(
            offsetEuler.x,
            offsetEuler.y,
            offsetEuler.z
        );

        initialRot = Quaternion.Euler(
            initialEuler.x,
            initialEuler.y,
            initialEuler.z
        );

        // initialRot = this.transform.localRotation;
        initialTargetRotInv = Quaternion.Inverse(target.transform.localRotation);
    }

    // Update is called once per frame
    void Update()
    {
        initialRot = Quaternion.Euler(
            initialEuler.x,
            initialEuler.y,
            initialEuler.z
        );

        offsetRot = Quaternion.Euler(
            offsetEuler.x,
            offsetEuler.y,
            offsetEuler.z
        );

        Quaternion diffRot = target.transform.localRotation * initialTargetRotInv;
        this.transform.rotation = offsetRot * diffRot * Quaternion.Inverse(offsetRot) * initialRot;

    }
}
