using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.InteropServices;
using System;
public class TestPlugin : MonoBehaviour
{
	[DllImport ("IcovRendererPlugin")]
	private static extern float SetTimeFromUnity(float t);


    public Text DebugText;
    // Start is called before the first frame update
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {
        DebugText.text = SetTimeFromUnity(Time.time).ToString();
    }
}
