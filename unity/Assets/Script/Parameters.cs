using System;
using System.Collections;
using UnityEngine;
using UnityEngine.UI;

public class Parameters : MonoBehaviour {

	public AnimationAvator animava;
	private IntPtr solver;


	// Use this for initialization
	void Start () {
		
		solver = animava.Solver;

	}
	
	// Update is called once per frame
	void Update () {
		

		GetComponent<Text>().text = 
		"Kernel Lengthscale: \t"+ VRIKSolver.GetKernelLengthScale(solver).ToString("f") + "\n" + 
		"Kernel Variance: \t\t" + VRIKSolver.GetKernelVariance(solver).ToString("f") + "\n" + 
		"Gaussian Variance: \t"+ VRIKSolver.GetGaussianVariance(solver).ToString("f") + "\n" +
		"IK Weight: \t\t\t\t\t"+ animava.IKWeight.ToString("f");

	}
}
