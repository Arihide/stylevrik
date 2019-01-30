using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class UI : MonoBehaviour {

	AnimationAvator animava;
	public GameObject avator;


	// Use this for initialization
	void Start () {
		
		animava = avator.GetComponent<AnimationAvator>();

	}
	
	// Update is called once per frame
	void Update () {
		

		GetComponent<Text>().text = 
		"Kernel Lengthscale: \t"+ animava.KernelLengthScale.ToString("f") + "\n" + 
		"Kernel Variance: \t\t" + animava.KernelVariance.ToString("f") + "\n" + 
		"Gaussian Variance: \t"+ animava.GaussianVariance.ToString("f") + "\n" + 
		"IK Weight: \t\t\t\t\t"+ animava.IKWeight.ToString("f");

	}
}
