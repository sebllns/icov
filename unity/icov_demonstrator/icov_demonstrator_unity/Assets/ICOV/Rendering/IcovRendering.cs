using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.InteropServices;
using System;
using AOT;

public class IcovRendering : MonoBehaviour
{

    //Create string param callback delegate
    delegate void DebugCallback(IntPtr request, int size);


    // Native plugin rendering events are only called if a plugin is used
    // by some script. This means we have to DllImport at least
    // one function in some active script.
    // For this example, we'll call into plugin's SetTimeFromUnity
    // function and pass the current time so the plugin can animate.
#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void SetTimeFromUnity(float t);


#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void SetPoVFromUnity(float x, float y);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void InitFullControllerFromUnity();

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void InitLoggerFromUnity(DebugCallback cb);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void DestroyDecoderFromUnity();


    // We'll also pass native pointer to a texture in Unity.
    // The plugin will fill texture data from native code.
#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void SetTextureFromUnity(System.IntPtr texture, int w, int h);


#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern IntPtr GetRenderEventFunc();

#if UNITY_WEBGL && !UNITY_EDITOR
	[DllImport ("__Internal")]
	private static extern void RegisterPlugin();
#endif



    public InteractionBase Pov;

    public Camera MainCamera;
    public Material RenderingMaterial;

    public RawImage FinalTexture;


	private IcovFileLogHandler m_logHandler;


	void Awake()
	{
		m_logHandler = new IcovFileLogHandler("icov.log");
	}

    // Start is called before the first frame update
    IEnumerator Start()
    {
#if UNITY_WEBGL && !UNITY_EDITOR
		RegisterPlugin();
#endif



        InitLoggerFromUnity(OnDebugCallback);
        InitFullControllerFromUnity();
        CreateTextureAndPassToPlugin();
        yield return StartCoroutine("CallPluginAtEndOfFrames");
    }

    //[MonoPInvokeCallback(typeof(DebugCallback))]
    static void OnDebugCallback(IntPtr request, int size)
    {
        //Ptr to string
        string debug_string = Marshal.PtrToStringAnsi(request, size-1);
        UnityEngine.Debug.Log(debug_string);
    }

    void Update()
    {
        RenderingMaterial.SetFloat("_CameraVFov", MainCamera.fieldOfView);
        RenderingMaterial.SetFloat("_CameraHFov", Camera.VerticalToHorizontalFieldOfView(MainCamera.fieldOfView, MainCamera.aspect));

        Vector3 rot = Pov.Angle;
        RenderingMaterial.SetFloat("_PovAngleY", rot.y);
        RenderingMaterial.SetFloat("_PovAngleX", rot.x);
    }

    private void CreateTextureAndPassToPlugin()
    {
        //int width = Screen.width;
        //int height = Screen.height;

        int width = RenderingMaterial.mainTexture.width;
        int height = RenderingMaterial.mainTexture.height;

        // Create a texture
        Texture2D tex = new Texture2D(width, height, TextureFormat.ARGB32, false);
        // Call Apply() so it's actually uploaded to the GPU
        tex.Apply();


        // Set texture onto our material
        FinalTexture.texture = tex;


        // Pass texture pointer to the plugin
        SetTextureFromUnity(tex.GetNativeTexturePtr(), tex.width, tex.height);
    }

    void OnDestroy()
    {
        DestroyDecoderFromUnity();
		RenderingMaterial.SetFloat("_PovAngleY", 0);
        RenderingMaterial.SetFloat("_PovAngleX", 0);
    }

    private IEnumerator CallPluginAtEndOfFrames()
    {
        while (true)
        {
            // Wait until all frame rendering is done
            yield return new WaitForEndOfFrame();

            // Set time for the plugin
            SetTimeFromUnity(Time.timeSinceLevelLoad);

            Vector3 rot = Pov.Angle;

            SetPoVFromUnity(rot.x, rot.y);

            // Issue a plugin event with arbitrary integer identifier.
            // The plugin can distinguish between different
            // things it needs to do based on this ID.
            // For our simple plugin, it does not matter which ID we pass here.
            GL.IssuePluginEvent(GetRenderEventFunc(), 1);
        }
    }
}
