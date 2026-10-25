using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Runtime.InteropServices;
using System;
using AOT;
using UnityEngine.Assertions;

public class IcovLightPlugin : MonoBehaviour
{
    public enum NeighborDirection
    {
        THIS    = 0,
        RIGHT    = 1,
        LEFT   ,
        UP     ,
        DOWN  ,
        COUNT,
    };

    public enum predType
    {
        NONE = 0,
        RIGHT = (1 << NeighborDirection.RIGHT),
        LEFT = (1 << NeighborDirection.LEFT),
        TOP = (1 << NeighborDirection.UP),
        BOTTOM = (1 << NeighborDirection.DOWN),
        TOP_RIGHT = (1 << NeighborDirection.UP) | (1 << NeighborDirection.RIGHT),
        TOP_LEFT = (1 << NeighborDirection.UP) | (1 << NeighborDirection.LEFT),
        BOTTOM_RIGHT = (1 << NeighborDirection.DOWN) | (1 << NeighborDirection.RIGHT),
        BOTTOM_LEFT = (1 << NeighborDirection.DOWN) | (1 << NeighborDirection.LEFT),
    };

    //Create string param callback delegate
    delegate void DebugCallback(IntPtr msg, int size);
    delegate void ProcessBlock(UInt32 block, float u, float v, float w, float h, UInt16 prediction);
    delegate void ProcessEvent();

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
    private static extern void InitLightControllerFromUnity(ProcessBlock pb, ProcessEvent start, ProcessEvent end, int videoWidth, int videoHeight, int block_size, int screenWidth, int screenHeight, float fovX_radian);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void GetAllBlocks(ProcessBlock pb);

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern UInt32 GetBlocksCount();

#if (UNITY_IOS || UNITY_TVOS || UNITY_WEBGL) && !UNITY_EDITOR
	[DllImport ("__Internal")]
#else
    [DllImport("IcovRendererPlugin")]
#endif
    private static extern void GetVisibleBlocks(out IntPtr data, out int size);

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

    //public InteractionBase Pov;

    public Camera MainCamera;

    public Material RenderingMaterial;

	private IcovFileLogHandler m_logHandler;

    public Texture2D m_blockShaderValues;
    private Color[] m_resetColorArray;
    private Color[] m_ColorBlocks;
    private Vector3 m_rotation;

	void Awake()
	{
		m_logHandler = new IcovFileLogHandler("icov.log");
	}

    // Start is called before the first frame update
    void Start()
    {
#if UNITY_WEBGL && !UNITY_EDITOR
		RegisterPlugin();
#endif
        int w = 8888;
        int h = 4444;
        int block_size = 202;

        InitLoggerFromUnity(OnDebugCallback);
        InitLightControllerFromUnity(ProcessBlockCallback, StartProcess, EndProcess, w, h, block_size, Screen.width, Screen.height, Mathf.Deg2Rad * Camera.VerticalToHorizontalFieldOfView(94.5864f, 16.0f/9.0f));
        UInt32 blockCount = GetBlocksCount();

        int block_cols = (int)Mathf.CeilToInt((float)w/block_size);
        int block_rows = (int)Mathf.CeilToInt((float)h/block_size);
        m_blockShaderValues = new Texture2D((int)blockCount, 1, TextureFormat.RGBA32, false, false);
        GetAllBlocks(InitBlockCallback);
        m_blockShaderValues.Apply(false);
        m_resetColorArray = m_blockShaderValues.GetPixels();
        m_ColorBlocks = m_blockShaderValues.GetPixels();
        RenderingMaterial.SetTexture("_BlockTex", m_blockShaderValues);
        RenderingMaterial.SetFloat("_BlockSizeX", (float)block_size/w);
        RenderingMaterial.SetFloat("_BlockSizeY", (float)block_size/h);

        m_rotation = Mathf.Deg2Rad * MainCamera.transform.eulerAngles;
        ProcessVisibleBlocks();
    }

    void InitBlockCallback(UInt32 block, float u, float v, float w, float h, UInt16 prediction)
    {
        m_blockShaderValues.SetPixel((int)block-1, 0, new Color(0, 0, 0, 0));
    } 

    void ProcessBlockCallback(UInt32 block, float u, float v, float w, float h, UInt16 prediction)
    {
        // Debug.Log("ProcessBlockCallback " + block + " " + u + " " + v + " " + w + " " + h + " " + prediction);
        m_ColorBlocks[(int)block-1] = (1.0f / (prediction + 1)) * Color.white;
    }

    void StartProcess()
    {
        // Debug.Log("StartProcess");
    }

    void EndProcess() 
    {
        // Debug.Log("EndProcess");
    }

    //[MonoPInvokeCallback(typeof(DebugCallback))]
    static void OnDebugCallback(IntPtr msg, int size)
    {
        try
        {
            //Ptr to string
            string debug_string = Marshal.PtrToStringAnsi(msg, size-1);
            UnityEngine.Debug.Log(debug_string);
        }
        catch(System.Exception e)
        {
            Debug.LogException(e);
        }
    }

    void Update()
    {
        m_resetColorArray.CopyTo(m_ColorBlocks, 0);
        Vector3 rot = Mathf.Deg2Rad * MainCamera.transform.eulerAngles;

        Debug.Log(rot);

        if(rot != m_rotation)
        {
            m_rotation = rot;
            ProcessVisibleBlocks();
        }
    }

    void ProcessVisibleBlocks()
    {
        SetPoVFromUnity(m_rotation.y, m_rotation.x);
        IntPtr data_ptr = IntPtr.Zero;
        int[] data = null;
        int count = 0;
        GetVisibleBlocks(out data_ptr, out count);
        Assert.IsTrue(count > 0 && count%2 == 0, "block data count should be even > 0 ; count = " + count);
        if(count > 0)
        {
            Color32 color = new Color32(0,0,0,0);
            // Load the results into a managed array.
            data = new int[count];
            Marshal.Copy(data_ptr, data, 0, count);
            for(int i = 0; i < count; i++)
            {
                int block = data[i];
                predType prediction = (predType)data[++i];
                switch(prediction)
                {
                    case predType.NONE:
                        color = new Color32(0, 255, 0, 255);
                        break;
                    case predType.RIGHT:
                        color = new Color32(0, 0, 255, 255);
                        break;
                    case predType.LEFT:
                        color = new Color32(255, 0, 0, 255);
                        break;
                    case predType.TOP:
                        color = new Color32(255, 255, 0, 255);
                        break;
                    case predType.BOTTOM:
                        color = new Color32(255, 0, 255, 255);
                        break;
                    case predType.TOP_RIGHT:
                        color = new Color32(0, 255, 255, 255);
                        break;
                    case predType.TOP_LEFT:
                        color = new Color32(64, 255, 127, 255);
                        break;
                    case predType.BOTTOM_RIGHT:
                        color = new Color32(127, 255, 64, 255);
                        break;
                    case predType.BOTTOM_LEFT:
                        color = new Color32(64, 124, 65, 255);
                        break;
                }
                
                m_ColorBlocks[(int)block-1] = color;
                //m_ColorBlocks[(int)block-1] = Color.cyan;
            }

            m_blockShaderValues.SetPixels(m_ColorBlocks);
            m_blockShaderValues.Apply(false);
        }


    }

    void OnDestroy()
    {
        DestroyDecoderFromUnity();
    }
}
