using System;
using UnityEngine;
using UnityEngine.UI;

public class LatentPosition : MonoBehaviour
{

    public AnimationAvator avator;
    public bool poseFromLatent = false;

    public Transform Point;
    public Text likelihood;

    private IntPtr solver;

    void Start()
    {

        solver = avator.Solver;

        int n = VRIKSolver.GetNumberOfData(solver);

        for (int i = 0; i < n; i++)
        {
            float x = VRIKSolver.GetLearnedLatentVariable(solver, i, 0);
            float y = VRIKSolver.GetLearnedLatentVariable(solver, i, 1);
            float z = VRIKSolver.GetLearnedLatentVariable(solver, i, 2);

            Vector3 point = new Vector3(x, y, z);

            var obj = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            obj.name = i.ToString();
            obj.GetComponent<Renderer>().material.color = Color.blue;
            obj.transform.parent = this.transform;
            obj.transform.localPosition = point * 1.0f;
            // obj.transform.localScale = new Vector3(1, 1, 1) * (float)sigma * 2 * 20.0f;
            obj.transform.localScale = new Vector3(1, 1, 1) * 0.04f;

        }
    }


    void Update()
    {

        if(poseFromLatent){

            VRIKSolver.Predict(solver, Point.localPosition.x, Point.localPosition.y, Point.localPosition.z);

        }
        
        Point.localPosition = new Vector3(
            VRIKSolver.GetLatentVariable(solver, 0),
            VRIKSolver.GetLatentVariable(solver, 1),
            VRIKSolver.GetLatentVariable(solver, 2)
        );

        likelihood.text = VRIKSolver.GetLikelihood(solver).ToString();


    }

}