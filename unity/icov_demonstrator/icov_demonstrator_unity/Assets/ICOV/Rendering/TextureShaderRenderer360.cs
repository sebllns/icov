using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.InteropServices;
using System;
using AOT;

public class TextureShaderRenderer360 : MonoBehaviour
{
    public InteractionBase Pov;
    public Camera MainCamera;
    public Material RenderingMaterial;

    void Update()
    {
        RenderingMaterial.SetFloat("_CameraVFov", MainCamera.fieldOfView);
        RenderingMaterial.SetFloat("_CameraHFov", Camera.VerticalToHorizontalFieldOfView(MainCamera.fieldOfView, MainCamera.aspect));

        Vector3 rot = Pov.Angle;
        RenderingMaterial.SetFloat("_PovAngleY", rot.y);
        RenderingMaterial.SetFloat("_PovAngleX", rot.x);
    }

    void OnDestroy()
    {
		RenderingMaterial.SetFloat("_PovAngleY", 0);
        RenderingMaterial.SetFloat("_PovAngleX", 0);
    }

}
