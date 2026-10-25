// Example low level rendering Unity plugin

#include "PlatformBase.h"
#include "RenderAPI.h"

#include <assert.h>
#include <math.h>
#include <vector>

#include "omnidirectional_layer/interface/IBlockDecoder.h"
#include "renderer/clientcontroller.h"
#include "renderer/LocalClientControllerLight.h"
#include "client/interface/client.h"
#include "tools/Logger.h"
#include "helpers.h"
static float g_Time;

static void *g_TextureHandle = NULL;
static int g_TextureWidth = 0;
static int g_TextureHeight = 0;

static IUnityInterfaces *s_UnityInterfaces = NULL;
static IUnityGraphics *s_Graphics = NULL;

static RenderAPI *s_CurrentAPI = NULL;
static UnityGfxRenderer s_DeviceType = kUnityGfxRendererNull;
static ClientController *s_Controller = NULL;
static LocalClientControllerLight *s_LightController = NULL;
static icov::Logger *s_logger = nullptr;
static icov::Logger *s_errlogger = nullptr;

// --------------------------------------------------------------------------
// SetTimeFromUnity

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTimeFromUnity(float t) { g_Time = t; }

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API InitLoggerFromUnity(icov::LogCallBack cb)
{
    s_logger = new icov::Logger(std::cout, true, false, icov::enum_t::LOG_TYPE_INFO, false);
	s_logger->redirect(cb);
	s_logger->logToFile("icov.log");

    s_errlogger = new icov::Logger(std::cerr, true, false, icov::enum_t::LOG_TYPE_INFO, false);
    s_errlogger->logLevel = icov::enum_t::LOG_TYPE_ERROR;
	s_errlogger->redirect(cb);
	s_errlogger->logToFile("icov_err.log");


	s_Controller = new ClientController();
	s_Controller->start();


}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API InitFullControllerFromUnity()
{
    s_Controller = new ClientController();
    s_Controller->start();
}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API InitLightControllerFromUnity(ProcessBlock pb, ProcessEvent start, ProcessEvent end, int videoWidth, int videoHeight, int block_size, int screenWidth, int screenHeight, float fovX_radian)
{
    s_LightController = new LocalClientControllerLight(pb, start, end, videoWidth, videoHeight, block_size, screenWidth, screenHeight, fovX_radian);
    s_LightController->start();
}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API DestroyDecoderFromUnity()
{
	cout << "DestroyDecoderFromUnity()" << endl;
    if(s_Controller) delete s_Controller;
    if(s_LightController) delete s_LightController;
    if(s_logger) delete s_logger;
    if(s_errlogger) delete s_errlogger;
	cout << "DONE DestroyDecoderFromUnity()" << endl;
}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetPoVFromUnity(float x, float y)
{
    try
    {
        if(s_LightController)
        {
            s_LightController->setPoV(x, y);
            s_LightController->update();
        }
        if(s_Controller) s_Controller->setPoV(x, y);
    }
    catch(...)
    {
        std::cerr << "\nFatal error" << std::endl;
        std::cerr << boost::current_exception_diagnostic_information() << std::endl;
        throw;
    }
}

// --------------------------------------------------------------------------
// SetTextureFromUnity

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetTextureFromUnity(void *textureHandle, int w, int h)
{
	// A script calls this at initialization time; just remember the texture pointer here.
	// Will update texture pixels each frame from the plugin rendering event (texture update
	// needs to happen on the rendering thread).
	g_TextureHandle = textureHandle;
	g_TextureWidth = w;
	g_TextureHeight = h;
}

// --------------------------------------------------------------------------
extern "C" uint32_t UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetBlocksCount()
{
    ICOV_ASSERT_MSG(s_LightController != nullptr, "Enabled only for the light mode");
    return s_LightController->getBlocksCount();
}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetAllBlocks(ProcessBlock pb)
{
    ICOV_ASSERT_MSG(s_LightController != nullptr, "Enabled only for the light mode");
    s_LightController->getAllBlocks(pb);
}

// --------------------------------------------------------------------------
extern "C" void UNITY_INTERFACE_EXPORT GetVisibleBlocks(int* &blocks , int &count)
{
    ICOV_ASSERT_MSG(s_LightController != nullptr, "Enabled only for the light mode");
    s_LightController->getVisibleBlocks(blocks, count);
}

// --------------------------------------------------------------------------
// UnitySetInterfaces

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType);

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces *unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

#if SUPPORT_VULKAN
	if (s_Graphics->GetRenderer() == kUnityGfxRendererNull)
	{
		extern void RenderAPI_Vulkan_OnPluginLoad(IUnityInterfaces *);
		RenderAPI_Vulkan_OnPluginLoad(unityInterfaces);
	}
#endif // SUPPORT_VULKAN

	// Run OnGraphicsDeviceEvent(initialize) manually on plugin load
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API UnityPluginUnload()
{
	s_Graphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

#if UNITY_WEBGL
typedef void(UNITY_INTERFACE_API *PluginLoadFunc)(IUnityInterfaces *unityInterfaces);
typedef void(UNITY_INTERFACE_API *PluginUnloadFunc)();

extern "C" void UnityRegisterRenderingPlugin(PluginLoadFunc loadPlugin, PluginUnloadFunc unloadPlugin);

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API RegisterPlugin()
{
	UnityRegisterRenderingPlugin(UnityPluginLoad, UnityPluginUnload);
}
#endif

// --------------------------------------------------------------------------
// GraphicsDeviceEvent

static void UNITY_INTERFACE_API OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	// Create graphics API implementation upon initialization
	if (eventType == kUnityGfxDeviceEventInitialize)
	{
		assert(s_CurrentAPI == NULL);
		s_DeviceType = s_Graphics->GetRenderer();
		s_CurrentAPI = CreateRenderAPI(s_DeviceType);
	}

	// Let the implementation process the device related events
	if (s_CurrentAPI)
	{
		s_CurrentAPI->ProcessDeviceEvent(eventType, s_UnityInterfaces);
	}

	// Cleanup graphics API implementation upon shutdown
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		delete s_CurrentAPI;
		s_CurrentAPI = NULL;
		s_DeviceType = kUnityGfxRendererNull;
	}
}

// --------------------------------------------------------------------------
// ModifyTexturePixels
static void ModifyTexturePixels()
{
	void *textureHandle = g_TextureHandle;
	int width = g_TextureWidth;
	int height = g_TextureHeight;
	if (!textureHandle)
		return;

	int textureRowPitch;
	void *textureDataPtr = s_CurrentAPI->BeginModifyTexture(textureHandle, width, height, &textureRowPitch);
	if (!textureDataPtr)
		return;

	const float t = g_Time * 4.0f;
	unsigned char *dst = (unsigned char *)textureDataPtr;

	const IRenderer &renderer = s_Controller->getClient().getDecoder().getRenderer();
    auto &decoder = s_Controller->getClient().getDecoder();

	decoder.setTexture(dst, width, height, textureRowPitch);

	s_Controller->update();

	s_CurrentAPI->EndModifyTexture(textureHandle, width, height, textureRowPitch, textureDataPtr);
}

// --------------------------------------------------------------------------
// OnRenderEvent
static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	// Unknown / unsupported graphics device type? Do nothing
	if (s_CurrentAPI == NULL)
		return;

	ModifyTexturePixels();
}

// --------------------------------------------------------------------------
// GetRenderEventFunc, an example function we export which is used to get a rendering event callback function.

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return OnRenderEvent;
}
