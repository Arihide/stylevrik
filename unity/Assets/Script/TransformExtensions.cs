using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public static class TransformExtensions
{

    public static Matrix4x4 GetLocalMatrix(this Transform transform, bool unitScale = true)
    {
        return Matrix4x4.TRS(transform.localPosition, transform.localRotation, unitScale ? Vector3.one : transform.localScale);
    }

    public static Matrix4x4 GetWorldMatrix(this Transform transform, bool unitScale = true)
    {
        return Matrix4x4.TRS(transform.position, transform.rotation, unitScale ? Vector3.one : transform.lossyScale);
    }

    public static void GetRotationArray(this Transform transform, float[] dist)
    {

        Matrix4x4 mat = Matrix4x4.TRS(Vector3.zero, transform.localRotation, Vector3.one);

        dist[0] = mat[0, 0];
        dist[1] = mat[1, 0];
        dist[2] = mat[2, 0];
        dist[3] = mat[0, 1];
        dist[4] = mat[1, 1];
        dist[5] = mat[2, 1];
        dist[6] = mat[0, 2];
        dist[7] = mat[1, 2];
        dist[8] = mat[2, 2];

    }

    public static void OverridePosition(this Transform transform, Vector3 position)
    {
        Transform[] childs = new Transform[transform.childCount];
        for (int i = 0; i < childs.Length; i++)
        {
            childs[i] = transform.GetChild(i);
        }
        transform.DetachChildren();
        transform.position = position;
        for (int i = 0; i < childs.Length; i++)
        {
            childs[i].SetParent(transform);
        }
    }

    public static void OverrideRotation(this Transform transform, Quaternion rotation)
    {
        Transform[] childs = new Transform[transform.childCount];
        for (int i = 0; i < childs.Length; i++)
        {
            childs[i] = transform.GetChild(i);
        }
        transform.DetachChildren();
        transform.rotation = rotation;
        for (int i = 0; i < childs.Length; i++)
        {
            childs[i].SetParent(transform);
        }
    }

}
