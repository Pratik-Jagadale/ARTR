// HEADER FILES
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "VK.h"

// VULKAN HEADERS
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

// VULKAN LIBRARY LINKING
#pragma comment(lib, "vulkan-1.lib")

#define WINWIDTH 800
#define WINHEIGHT 600

// GLOBAL FUNCTION DECLARATION
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void ToggleFullScreen();

// GLOBAL VARIABLE DECLARATIONS
const char *gpszAppName = "ARTR";

BOOL gbActiveWindow = FALSE;
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
FILE *gpFile = NULL;

// INSTANCE EXTENSION RELATED VARIABLES
uint32_t enabledInstanceExtensionCount = 0;
const char *enabledInstanceExtensionNames_Array[3]; // VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME & VK_EXT_DEBUG_REPORT_EXTENSION_NAME

// VULKAN INSTANCE
VkInstance vkInstance = VK_NULL_HANDLE;

// Vulkan Presentation Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

// VULKAN PHYSICAL DEVICE RELATED
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

// DEVICE EXTENSION RELATED VARIABLES
uint32_t enabledDeviceExtensionCount = 0;
const char *enabledDeviceExtensionNames_Array[1]; // VK_KHR_SWAPCHAIN_EXTENSION_NAME

// VK DEVICE
VkDevice vkDevice = VK_NULL_HANDLE;

// DEVICE QUEUE
VkQueue vkQueue = VK_NULL_HANDLE;

// COLOR FORMAT & COLOR SPACE
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

// PRESENTATION MODE
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

// SWAPCHAIN
VkSwapchainKHR vkSwapChainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

int winWidth = WINWIDTH;
int winHeight = WINHEIGHT;

// SWAPN CHAIN IMAGE AND SWAP CHAIN IMAGE VIEW
uint32_t swapChainImageCount = UINT32_MAX;
VkImage *swapChainImage_Array = NULL;
VkImageView *vkSwapChainImageViews_Array = NULL;

// COMMAND POOL
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

// COMMAND BUFFER
VkCommandBuffer *vkCommandBuffer_Array = NULL;

// RENDER PASS
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

// FRAMEBUFFERS
VkFramebuffer *vkFramebuffer_Array = NULL;

// SYNCRONISATION
VkSemaphore vkSemaphore_BackBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_RenderComplete = VK_NULL_HANDLE;
VkFence *vkFence_Array = NULL;

// COMMAND BUFFERS - CLEAR COLOR
VkClearColorValue vkClearColorValue;

// RENDER
BOOL bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

// VALIDATION LAYER
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char *enabledValidationLayerNames_Array[1]; // FOR VK_LAYER_KHRONOUSE_VALIDATION
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

// VERTEX BUFFER RELATED VARIABLE
typedef struct
{
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
} VertexData;

VertexData vertexData_position;

// SHADER RELATED VARIABLES
VkShaderModule vkShaderModule_Vertex = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_Fragment = VK_NULL_HANDLE;

// descriptor set layout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

// pipeline layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

// PIPELINE
VkViewport vkViewport;
VkRect2D vkRect2D_Scissor;
VkPipeline vkPipeline;

// ENTRY POINT FUNCTION
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// FUNCTION DECLARATIONS
	VkResult initialize(void);
	void uninitialize(void);
	VkResult display(void);
	void update(void);

	// VARIABLE DECLARATIONS
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	BOOL bDone = FALSE;
	VkResult vkResult = VK_SUCCESS;

	int iRetVal = 0;
	int iHeightOfWindow;
	int iWidthOfWindow;
	int iXMyWindow;
	int iYMyWindow;

	// CREATE LOG FILE
	gpFile = fopen("Log.txt", "w");
	if (!gpFile)
	{
		MessageBox(NULL, TEXT("CREATION OF LOG FILE FAILED..!!! EXITING..."), TEXT("FILE I/O ERROR"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "%s => LOG FILE IS SUCCESSFULLY CREATED.\n", __func__);
	}

	wsprintf(szAppName, TEXT("%s"), gpszAppName);

	// INITIALIZATION OF WNDCLASSEX STRUCTURE
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// REGISTER THE ABOVE wndclass
	RegisterClassEx(&wndclass);

	iHeightOfWindow = GetSystemMetrics(SM_CYSCREEN); // HEIGHT OF WINDOW SCREEN
	iWidthOfWindow = GetSystemMetrics(SM_CXSCREEN);	 // WIDTH OF WINDOW SCREEN

	iXMyWindow = (iWidthOfWindow) / 4;	// X COORDINATE FOR MyWindow
	iYMyWindow = (iHeightOfWindow) / 4; // Y COORDINATE FOR MyWindow

	// CREATE WINDOW
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
						  szAppName,
						  TEXT("Vulkan"),
						  WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
						  iXMyWindow,
						  iYMyWindow,
						  (iWidthOfWindow) / 2,
						  (iHeightOfWindow) / 2,
						  NULL,
						  NULL,
						  hInstance,
						  NULL);

	ghwnd = hwnd;

	// INITIALIZE
	vkResult = initialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => initialize()  IS FAILED.\n", __func__);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(gpFile, "%s => initialize() IS SUCCEEDED.\n", __func__);
	}

	ShowWindow(hwnd, iCmdShow);

	// FOREGROUNDING AND FOCUSING WINDOW
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// GAME LOOP
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = TRUE;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == TRUE)
			{
				// RENDER THE SCENE
				display();

				// UPDATE THE SCENE
				update();
			}
		}
	}

	uninitialize();
	return ((int)msg.wParam);
}

// CALLBACK FUNCTION
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// FUNCTION DECLARATIONS
	void resize(int, int);
	void ToggleFullScreen();

	// CODE
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;
		break;

	case WM_ERASEBKGND:
		break;

	case WM_CHAR:
		switch (wParam)
		{
		case 'f':
		case 'F':
			ToggleFullScreen();
			break;
		case 27:
			PostQuitMessage(0);
		}
		break;

	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		break;
	}
	return (DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullScreen()
{
	// VARIABLE DECLARATIONS
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// CODE
	wp.length = sizeof(WINDOWPLACEMENT);
	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
	}
}

VkResult initialize(void)
{
	// FUNCTION DECLARATIONS
	VkResult createVulkanInstance(void);
	VkResult getSupportedSurface(void);
	VkResult getPhysicalDevice(void);
	VkResult printVKInfo(void);
	VkResult fillDeviceExtensionNames(void);
	VkResult createVulkanDevice(void);
	void getDeviceQueue(void);
	VkResult getPhysicalDeviceSurfaceFormateAndColorSpace(void);
	VkResult getPhysicalDevicePresentMode(void);
	VkResult createSwapchain(VkBool32);
	VkResult createImagesAndImageViews(void);
	VkResult createCommandPool(void);
	VkResult createCommandBuffers(void);
	VkResult createVertexBuffer(void);
	VkResult createShader(void);
	VkResult createDescriptorSetLayout(void);
	VkResult createPipelineLayout(void);
	VkResult createRenderPass(void);
	VkResult createPipeline(void);
	VkResult createFrameBuffer(void);
	VkResult createSemaphore(void);
	VkResult createFences(void);
	VkResult buildCommandBuffers(void);

	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	fprintf(gpFile, "===========================================================================================\n");
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createVulkanInstance() IS FAILED.\n", __func__);
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	else
	{
		fprintf(gpFile, "%s => createVulkanInstance() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE VULKAN PRESENTATION SURFACE
	vkResult = getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => getSupportedSurface() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => getSupportedSurface() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// PHYSICAL DEVICE
	vkResult = getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => getPhysicalDevice() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => getPhysicalDevice() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// PRINT VK INFO
	vkResult = printVKInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => printVKInfo() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => printVKInfo() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// VULKAN DEVICE
	vkResult = createVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createVulkanDevice() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createVulkanDevice() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	//  DEVICE QUEUE
	getDeviceQueue();

	// COLOR FORMAT AND COLOR SPACE
	vkResult = getPhysicalDeviceSurfaceFormateAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => getPhysicalDeviceSurfaceFormateAndColorSpace() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => getPhysicalDeviceSurfaceFormateAndColorSpace() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE SWAPCGHAIN
	vkResult = createSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createSwapchain() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createSwapchain() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE VULKAN IMAGES AND IMAGEVIES
	vkResult = createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createImagesAndImageViews() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createImagesAndImageViews() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE COMMAND POOL
	vkResult = createCommandPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createCommandPool() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createCommandPool() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE COMMAND BUFEFRS
	vkResult = createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createCommandBuffers() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createCommandBuffers() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE VERTEX BUFFER
	vkResult = createVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createVertexBuffer() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createVertexBuffer() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE SHADER
	vkResult = createShader();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createShader() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createShader() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// DESRIPTOR SET LAYOUT
	vkResult = createDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createDescriptorSetLayout() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createDescriptorSetLayout() IS SUCCEEDED.\n", __func__);
	}

	fprintf(gpFile, "===========================================================================================\n");

	// PIPELINE LAYOUT
	vkResult = createPipelineLayout();
	if (vkResult != VK_SUCCESS)
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => createPipelineLayout() IS FAILED.\n", __func__);
			vkResult = VK_ERROR_INITIALIZATION_FAILED;
		}
		else
		{
			fprintf(gpFile, "%s => createPipelineLayout() IS SUCCEEDED.\n", __func__);
		}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE RENDERPASS
	vkResult = createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createRenderPass() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createRenderPass() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE PIPELINE
	vkResult = createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createPipeline() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createPipeline() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE FRAMEBUFER
	vkResult = createFrameBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createFrameBuffer() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createFrameBuffer() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE SEMAPHORE
	vkResult = createSemaphore();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createSemaphore() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createSemaphore() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// CREATE FENCES
	vkResult = createFences();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => createFences() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => createFences() IS SUCCEEDED.\n", __func__);
	}
	fprintf(gpFile, "===========================================================================================\n");

	// INTIALIZE CLEAR COLOR VALUE
	memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

	vkClearColorValue.float32[0] = 0.0f;
	vkClearColorValue.float32[1] = 0.0f;
	vkClearColorValue.float32[2] = 1.0f;
	vkClearColorValue.float32[3] = 1.0f;

	vkResult = buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => buildCommandBuffers() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => buildCommandBuffers() IS SUCCEEDED.\n", __func__);
	}

	fprintf(gpFile, "===========================================================================================\n");

	// INITALIZATION IS COMPLETED
	bInitialized = TRUE;

	return (vkResult);
}

void resize(int width, int height)
{
	// CODE
	if (height == 0)
		height = 1;
}

const char *vkResultToString(VkResult result)
{
	switch (result)
	{
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_FRAGMENTATION_EXT:
		return "VK_ERROR_FRAGMENTATION_EXT";
	case VK_ERROR_NOT_PERMITTED_EXT:
		return "VK_ERROR_NOT_PERMITTED_EXT";
	default:
		return "UNKNOWN_VK_RESULT";
	}
}

VkResult display(void)
{
	// LOCAL VARIABLES
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// IF CONTROL COMES HERE BEFORE INITIALIZATION GET COMPLETED RETURN FALSE
	if (bInitialized == FALSE)
	{
		fprintf(gpFile, "%s => INITIALIZATION YET NOT COMPLETED. HENCE RETURING FROM DISPLAY.\n", __func__);
		return (VkResult)VK_FALSE;
	}

	// ACQUIRE INDEX OF NEXT SWAPCHAIN IMAGE
	// IF NOT RETURS IN GIVEM TIME. IT WILL RETURN VK_NOT_READY
	vkResult = vkAcquireNextImageKHR(vkDevice, vkSwapChainKHR /*FROM WHERE*/, UINT64_MAX /*WAIT TIME OUT : NANO SEC*/, vkSemaphore_BackBuffer, VK_NULL_HANDLE, &currentImageIndex);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkAcquireNextImageKHR() IS FAILED %s.\n", __func__, vkResultToString(vkResult));
		return (vkResult);
	}

	// USE FENCE TO ALLOW TO HOST TO WAIT FOR COMPLETION OF EXECUTION OF PREVIOUS COMMAND BUFFER
	vkResult = vkWaitForFences(vkDevice, 1, &vkFence_Array[currentImageIndex], VK_TRUE, UINT64_MAX);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkWaitForFences() IS FAILED.\n", __func__);
		return (vkResult);
	}

	// READY THE FENCES FOR NEXT COMMAND BUFFER
	vkResult = vkResetFences(vkDevice, 1, &vkFence_Array[currentImageIndex]);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkResetFences() IS FAILED.\n", __func__);
		return (vkResult);
	}

	// ONE OF THE MEMBER OF VkSubmitInfo STRUCTURE REQUIRES ARRAY OF PIPELINE STAGES WE HAVE ONLY ONE OF COMPLETION OF COLOR ATTACHMENT OUTPUT STILL WE NEED ON MEMBER ARRAY.
	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// DECLARE, MEMSET & INITIAILIZE VkSubmitInfo Structure
	VkSubmitInfo vkSubmitInfo;

	memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &vkSemaphore_BackBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_Array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkSemaphore_RenderComplete;

	// SUBMIT ABOVE WORK TO THE QUEUE
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_Array[currentImageIndex]);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkQueueSubmit() IS FAILED.\n", __func__);
		return (vkResult);
	}

	// WE ARE GOING TO REPRENT RENDERD IMAGE AFTER DECLARING AND INITIALIZING VkPresentInfoKHR STRUCTURE
	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void *)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &vkSwapChainKHR;
	vkPresentInfoKHR.pImageIndices = &currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_RenderComplete;

	// PRESENT THE QUEUE
	vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkQueuePresentKHR() IS FAILED.\n", __func__);
		return (vkResult);
	}

	vkDeviceWaitIdle(vkDevice);

	return (vkResult);
}

void update(void)
{
	// CODE
}

void uninitialize(void)
{
	// FUNCTION DECLARATIONS
	void ToggleFullScreen(void);

	// CODE
	fprintf(gpFile, "===========================================================================================\n");

	if (gbFullScreen)
		ToggleFullScreen();

	// NO NEED TO DESTOY SELECTED PHYSICAL DEVICE
	// WAIT DEVICE IDLE
	if (vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "%s => vkDeviceWaitIdle is Done.\n", __func__);
	}

	// DESTROY FENCES
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		if (vkFence_Array)
		{
			vkDestroyFence(vkDevice, vkFence_Array[i], NULL);
			fprintf(gpFile, "%s => vkFence_Array[%d] DESTROYED SUCCESSFULLY.\n", __func__, i);
		}
	}
	if (vkFence_Array)
	{
		free(vkFence_Array);
		vkFence_Array = NULL;
		fprintf(gpFile, "%s => vkFence_Array FREE SUCCESSFULLY.\n", __func__);
	}

	// DESTROY SEMAPHORE
	if (vkSemaphore_RenderComplete)
	{
		vkDestroySemaphore(vkDevice, vkSemaphore_RenderComplete, NULL);
		fprintf(gpFile, "%s => vkSemaphore_RenderComplete DESTROYED SUCCESSFULLY.\n", __func__);
	}

	if (vkSemaphore_BackBuffer)
	{
		vkDestroySemaphore(vkDevice, vkSemaphore_BackBuffer, NULL);
		fprintf(gpFile, "%s => vkSemaphore_RenderComplete DESTROYED SUCCESSFULLY.\n", __func__);
	}

	// FREE FRAMEBUFFER
	if (vkFramebuffer_Array)
	{
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			if (vkFramebuffer_Array[i])
			{
				vkDestroyFramebuffer(vkDevice, vkFramebuffer_Array[i], NULL);
			}
		}

		free(vkFramebuffer_Array);
		vkFramebuffer_Array = NULL;
	}

	if (vkPipeline)
	{
		vkDestroyPipeline(vkDevice, vkPipeline, NULL);
		vkPipeline = VK_NULL_HANDLE;
	}

	// FREE RENDER PASS
	if (vkRenderPass)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
	}

	// DESTROY DEVICE MEMORY
	if (vertexData_position.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
		vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
	}

	// DESTROY VERTEX BUFFER
	if (vertexData_position.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
		vertexData_position.vkBuffer = VK_NULL_HANDLE;
	}

	// DESTROY SHADER MOUDLE
	if (vkShaderModule_Vertex)
	{
		vkDestroyShaderModule(vkDevice, vkShaderModule_Vertex, NULL);
		vkShaderModule_Vertex = VK_NULL_HANDLE;
	}

	if (vkShaderModule_Fragment)
	{
		vkDestroyShaderModule(vkDevice, vkShaderModule_Fragment, NULL);
		vkShaderModule_Fragment = VK_NULL_HANDLE;
	}

	if (vkPipelineLayout)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
	}

	// destroy descriptor set layout
	if (vkDescriptorSetLayout)
	{
		vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
		vkDescriptorSetLayout = VK_NULL_HANDLE;
	}

	// FREE COMMAND BUFFER ARRAY MEMBERS
	if (vkCommandBuffer_Array)
	{
		for (uint32_t i = 0; i < swapChainImageCount; i++)
		{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_Array[i]);
			fprintf(gpFile, "%s => vkCommandBuffer_Array[%d] DESTROYED SUCCESSFULLY.\n", __func__, i);
		}
	}

	// FREE COMMAND BUFFER
	if (vkCommandBuffer_Array)
	{
		free(vkCommandBuffer_Array);
		vkCommandBuffer_Array = NULL;
		fprintf(gpFile, "%s => vkCommandBuffer_Array FREE SUCCESSFULLY.\n", __func__);
	}

	// DESTROY COMMAND POOL
	if (vkCommandPool)
	{
		vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
		vkCommandPool = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vkCommandPool DESTROYED SUCCESSFULLY.\n", __func__);
	}

	// DON'T DESTROY THE IMAGES EXPLICITELY vkDestroySwapchainKHR WILL DESTROY THE IMAGES.

	// // FREE SWAP CHAIN IMAGES ARRAY MEMBER
	// for (uint32_t i = 0; i < swapChainImageCount; i++)
	// {
	// 	vkDestroyImage(vkDevice, swapChainImage_Array[i], NULL);
	// 	fprintf(gpFile, "%s => swapChainImage_Array[%d] DESTROYED SUCCESSFULLY.\n", __func__, i);
	// }

	// // FREE SWAP CHAIN IMAGES
	// if (swapChainImage_Array)
	// {
	// 	free(swapChainImage_Array);
	// 	swapChainImage_Array = NULL;
	// 	fprintf(gpFile, "%s => swapChainImage_Array FREE SUCCESSFULLY.\n", __func__);
	// }

	// FREE SWAPCHAIN IMAGE VIEWS ARRAY MEMBER
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkDestroyImageView(vkDevice, vkSwapChainImageViews_Array[i], NULL);
		fprintf(gpFile, "%s => vkSwapChainImageViews_Array[%d] DESTROYED SUCCESSFULLY.\n", __func__, i);
	}

	// FREE SWAPCHAIN IMAGE VIEWS
	if (vkSwapChainImageViews_Array)
	{
		free(vkSwapChainImageViews_Array);
		vkSwapChainImageViews_Array = NULL;
		fprintf(gpFile, "%s => vkSwapChainImageViews_Array FREE SUCCESSFULLY.\n", __func__);
	}

	// DESTROY SWAP CHAIN
	if (vkSwapChainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapChainKHR, NULL);
		vkSwapChainKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vkDestroySwapchainKHR() Succeeded\n", __func__);
	}

	// DESTORY VULAKN DEVICE
	if (vkDevice)
	{
		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vkDevice DESTROYED SUCCESSFULLY.\n", __func__);
	}

	// DESTROY VULKAN PRESENTATION SURFACE
	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL); // IT'S NOR PLATFORM SPECIFIC
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => VULKAN PRESENTATION SURFACE DESTROYED SUCCESSFULLY.\n", __func__);
	}

	//
	if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
	{
		vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
		vkDebugReportCallbackEXT = VK_NULL_HANDLE;
		vkDestroyDebugReportCallbackEXT_fnptr = NULL;
		fprintf(gpFile, "%s => vkDebugReportCallbackEXT SUCCESSFULLY.\n", __func__);
	}

	// DESTROY VULKAN INSTANCE
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => VULKAN INSTANCE DESTROYED SUCCESSFULLY.\n", __func__);
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		fprintf(gpFile, "%s => DESTROY WINDOW DESTROYED SUCCESSFULLY.\n", __func__);
	}

	if (gpFile)
	{
		fprintf(gpFile, "%s => LOG FILE SUCCESSFULLY CLOSED.\n", __func__);
		fprintf(gpFile, "===========================================================================================\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

////////////////////////////////////////////// VULKAN RELATED FUNCTIONS //////////////////////////////////////////////

VkResult createVulkanInstance(void)
{
	// FUNCTION DECLARTION
	VkResult fillInstanceExtensionNames(void);
	VkResult fillValidationLayerNames(void);
	VkResult createValidationCallbackFunction(void);

	// LOCAL VARIABLES
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// FILL INSTANCE EXTENSIONS
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => fillInstanceExtensionNames() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => fillInstanceExtensionNames() IS SUCCEEDED.\n", __func__);
	}

	// FILL VALIDATION LAYER NAMES
	if (bValidation == TRUE)
	{
		vkResult = fillValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => fillValidationLayerNames() IS FAILED.\n", __func__);
		}
		else
		{
			fprintf(gpFile, "%s => fillValidationLayerNames() IS SUCCEEDED.\n", __func__);
		}
	}

	// Initialize struct VkApplicationInfo
	VkApplicationInfo vkApplicationInfo;

	memset((void *)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));
	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // STRUCTURE TYPE
	vkApplicationInfo.pNext = NULL;								  // ONE STRUCT TO NEXT STRUCT;
	vkApplicationInfo.pApplicationName = gpszAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszAppName;
	vkApplicationInfo.engineVersion = 1;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;

	// Initialize struct VkInstaceCreateInfo
	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void *)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));

	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_Array;
	if (bValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_Array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}

	// Call vkCreateInstance function to get VkInstance in a global variable and do error checking.
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "%s => vkCreateInstance() IS FAILED - INCOMPATIBLE_DRIVER %d.\n", __func__, vkResult);
		return (vkResult);
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "%s => vkCreateInstance() IS FAILED - EXTENSION_NOT_PRESENT %d.\n", __func__, vkResult);
		return vkResult;
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateInstance() IS FAILED %d.\n", __func__, vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateInstance() IS SUCCEDED.\n", __func__);
	}

	// DO FOR VALIDATION CALL BACKS
	if (bValidation == TRUE)
	{
		vkResult = createValidationCallbackFunction();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => createValidationCallbackFunction() IS FAILED.\n", __func__);
		}
		else
		{
			fprintf(gpFile, "%s => createValidationCallbackFunction() IS SUCCEEDED.\n", __func__);
		}
	}

	return (vkResult);
}

VkResult fillInstanceExtensionNames(void)
{
	uint32_t instanceExtensionCount = 0;
	VkResult vkResult = VK_SUCCESS;

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => Extension Count : %d.\n", __func__, instanceExtensionCount);

		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceExtensionProperties() IS SUCCEEDED.\n", __func__);
	}

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceExtensionProperties() IS SUCCEEDED.\n", __func__);
	}

	char **instanceExtensionNames_Array = NULL;
	instanceExtensionNames_Array = (char **)malloc(sizeof(char *) * instanceExtensionCount);

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		instanceExtensionNames_Array[i] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(instanceExtensionNames_Array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "%s => Vulakn Instance Extension Name = %s.\n", __func__, instanceExtensionNames_Array[i]);
	}

	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtensionFound = VK_FALSE;
	VkBool32 debugReportExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_Array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_Array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			win32SurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_Array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;
			if (bValidation == TRUE)
			{
				enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else
			{
				// ARRAY WILL NOT HAVE ENTRY OF VK_EXT_DEBUG_REPORT_EXTENSION_NAME
			}
		}
	}

	// FREE ALLOCATED MEMORY
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (instanceExtensionNames_Array[i])
		{
			free(instanceExtensionNames_Array[i]);
			instanceExtensionNames_Array[i] = NULL;
		}
	}

	if (instanceExtensionNames_Array)
	{
		free(instanceExtensionNames_Array);
		instanceExtensionNames_Array = NULL;
	}

	// DISPLAY SUPPORTED EXTENSION NAME.
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => VK_KHR_SURFACE_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => VK_KHR_SURFACE_EXTENSION_NAME FOUND.\n", __func__);
	}

	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => VK_KHR_WIN32_SURFACE_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => VK_KHR_WIN32_SURFACE_EXTENSION_NAME FOUND.\n", __func__);
	}

	if (debugReportExtensionFound == VK_FALSE)
	{
		if (bValidation == TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED; //
			fprintf(gpFile, "%s => VALIDATION IS ON BUT REQUIRED VK_EXT_DEBUG_REPORT_EXTENSION_NAME EXTENSION IS NOT SUPPORTED.\n", __func__);

			return (vkResult);
		}
		else
		{
			fprintf(gpFile, "%s => VALIDATION IS OFF & REQUIRED VK_EXT_DEBUG_REPORT_EXTENSION_NAME EXTENSION IS NOT SUPPORTED.\n", __func__);
		}
	}
	else
	{
		if (bValidation == TRUE)
		{
			fprintf(gpFile, "%s => VALIDATION IS ON & VK_EXT_DEBUG_REPORT_EXTENSION_NAME IS ALSO FOUND.\n", __func__);
		}
		else
		{
			fprintf(gpFile, "%s => VALIDATION IS OFF & VK_EXT_DEBUG_REPORT_EXTENSION_NAME IS FOUND.\n", __func__);
		}
	}

	fprintf(gpFile, "===========================================================================================\n");
	// Print only supported extension names.
	for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
	{
		fprintf(gpFile, "%s => Enabled Vulkan Instance Extension NAMES = %s.\n", __func__, enabledInstanceExtensionNames_Array[i]);
	}

	return (vkResult);
}

VkResult fillValidationLayerNames(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;
	uint32_t validationLayerCount = 0;

	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateInstanceLayerProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceLayerProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => Validation Layer Count : %d.\n", __func__, validationLayerCount);

		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateInstanceLayerProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceLayerProperties() IS SUCCEEDED.\n", __func__);
	}

	VkLayerProperties *vkLayerProperties_Array = NULL;

	vkLayerProperties_Array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);

	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_Array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateInstanceLayerProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceLayerProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateInstanceLayerProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateInstanceLayerProperties() IS SUCCEEDED.\n", __func__);
	}

	char **validationLayerNames_Array = NULL;
	validationLayerNames_Array = (char **)malloc(sizeof(char *) * validationLayerCount);

	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		validationLayerNames_Array[i] = (char *)malloc(sizeof(char) * (strlen(vkLayerProperties_Array[i].layerName)) + 1);
		memcpy(validationLayerNames_Array[i], vkLayerProperties_Array[i].layerName, strlen(vkLayerProperties_Array[i].layerName) + 1);

		fprintf(gpFile, "%s => Vulakn Instance Layer Name = %s.\n", __func__, validationLayerNames_Array[i]);
	}

	if (vkLayerProperties_Array)
	{
		free(vkLayerProperties_Array);
		vkLayerProperties_Array = NULL;
	}

	VkBool32 validationLayerFound = VK_FALSE;

	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (strcmp(validationLayerNames_Array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			enabledValidationLayerNames_Array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}

	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (validationLayerNames_Array[i])
		{
			free(validationLayerNames_Array[i]);
			validationLayerNames_Array[i] = NULL;
		}
	}

	if (validationLayerNames_Array)
	{
		free(validationLayerNames_Array);
		validationLayerNames_Array = NULL;
	}

	if (validationLayerFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => VK_LAYER_KHRONOS_validation NOT SUPPORTED.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => VK_LAYER_KHRONOS_validation IS SUPPORTED.\n", __func__);
	}

	fprintf(gpFile, "===========================================================================================\n");

	for (uint32_t i = 0; i < enabledValidationLayerCount; i++)
	{
		fprintf(gpFile, "%s => Enable Vulkan Instance Validation Layer NAMES = %s.\n", __func__, enabledValidationLayerNames_Array[i]);
	}

	return (vkResult);
}

VkResult createValidationCallbackFunction(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char *, const char *, void *);

	// CODE
	// GET THE REQUIRED FUNCTION POINTERS
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");
	if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => vkGetInstanceProcAddr FAILED TO GET FUNCTION POINTER FOR vkCreateDebugReportCallbackEXT.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkGetInstanceProcAddr SUCCEDED TO GET FUNCTION POINTER FOR vkCreateDebugReportCallbackEXT.\n", __func__);
	}

	vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => vkGetInstanceProcAddr FAILED TO GET FUNCTION POINTER FOR vkDestroyDebugReportCallbackEXT.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkGetInstanceProcAddr SUCCEDED TO GET FUNCTION POINTER FOR vkDestroyDebugReportCallbackEXT.\n", __func__);
	}

	// GET THE VULKAN DEBUG REPORT CALL BACK OBJECT
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void *)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateDebugReportCallbackEXT_fnptr IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateDebugReportCallbackEXT_fnptr IS SUCCEDED.\n", __func__);
	}

	return (vkResult);
}

VkResult getSupportedSurface(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
	memset((void *)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE); // also we can use (HINSTANCE)GetModule(NULL)
	vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateWin32SurfaceKHR()  IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateWin32SurfaceKHR() IS SUCCEEDED.\n", __func__);
	}
	return (vkResult);
}

VkResult getPhysicalDevice(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkEnumeratePhysicalDevices() : 1 IS FAILED.\n", __func__);
		return (vkResult);
	}
	else if (physicalDeviceCount == 0)
	{
		fprintf(gpFile, "%s => physicalDeviceCount IS ZERO.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkEnumeratePhysicalDevices() : 1 IS SUCCEEDED.\n", __func__);
	}

	// Allocate VkQueueFamilyProperties Array according to above count.
	vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkEnumeratePhysicalDevices() : 2 IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkEnumeratePhysicalDevices() : 2 IS SUCCEEDED.\n", __func__);
	}

	VkBool32 bFound = VK_FALSE;

	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX; // IF PHYSICAL DEVICE PRESEN IT MUST PRESENT AT LEAST ONE QUEUE FAMILY
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);

		VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * queueCount);

		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);
		VkBool32 *isQueueSurfaceSupported_array = NULL;

		isQueueSurfaceSupported_array = (VkBool32 *)malloc(sizeof(VkBool32 *) * queueCount);

		for (uint32_t j = 0; j < queueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
		}

		for (uint32_t j = 0; j < queueCount; j++)
		{
			if (vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if (isQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					vkPhysicalDevice_selected = vkPhysicalDevice_array[i];
					graphicsQueueFamilyIndex_selected = j;
					bFound = VK_TRUE;
					break;
				}
			}
		}

		if (isQueueSurfaceSupported_array)
		{

			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;

			fprintf(gpFile, "%s => SUCCEDED TO FREE isQueueSurfaceSupported_array.\n", __func__);
		}

		if (vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;

			fprintf(gpFile, "%s => SUCCEDED TO FREE vkQueueFamilyProperties_array.\n", __func__);
		}

		if (bFound == VK_TRUE)
			break;
	}

	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "%s => getPhysicalDevice IS SUCCEDED TO GET PHYSICAL DEVICE.\n", __func__);
	}
	else
	{
		if (vkPhysicalDevice_array)
		{
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
		}

		fprintf(gpFile, "%s => getPhysicalDevice IS FAIELD TO GET PHYSICAL DEVICE.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	memset((void *)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, &vkPhysicalDeviceMemoryProperties);

	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void *)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

	vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);

	if (vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpFile, "%s => SELECTED DEVICE SUPPORTS TESSELATION SHADER.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => SELECTED DEVICE NOT SUPPORTS TESSELATION SHADER.\n", __func__);
	}

	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "%s => SELECTED DEVICE SUPPORTS GEOMETRY SHADER.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => SELECTED DEVICE NOT SUPPORTS GEOMETRY SHADER.\n", __func__);
	}

	return (vkResult);
}

VkResult printVKInfo(void)
{
	// VARIABLE DECLARATIONS

	// CODE
	fprintf(gpFile, "\n============================ Print Vulkan Info ======================\n");

	for (uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
		memset((void *)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));

		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);

		fprintf(gpFile, "API VERSION : %d.%d.%d.\n", (uint32_t)majorVersion, (uint32_t)minorVersion, (uint32_t)patchVersion);
		fprintf(gpFile, "DEVICE NAME : %s.\n", vkPhysicalDeviceProperties.deviceName);

		switch (vkPhysicalDeviceProperties.deviceType)
		{
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			fprintf(gpFile, "DEVICE TYPE : INTEGRATED GPU (iGPU).\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			fprintf(gpFile, "DEVICE TYPE : DISCRETE GPU (dGPU).\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			fprintf(gpFile, "DEVICE TYPE : VIRTUAL GPU (vGPU).\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			fprintf(gpFile, "DEVICE TYPE : CPU.\n");
			break;

		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			fprintf(gpFile, "DEVICE TYPE : OTHER.\n");
			break;

		default:
			fprintf(gpFile, "DEVICE TYPE : UNKNOWN.\n");
			break;
		}

		fprintf(gpFile, "VENDOR ID : 0x%04x.\n", vkPhysicalDeviceProperties.vendorID);
		fprintf(gpFile, "DEVICE ID : 0x%04x.\n", vkPhysicalDeviceProperties.deviceID);
	}

	if (vkPhysicalDevice_array)
	{
		fprintf(gpFile, "%s => vkPhysicalDevice_array DESTROYED.\n", __func__);
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
	}

	return VK_SUCCESS;
}

VkResult fillDeviceExtensionNames(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;
	uint32_t deviceExtensionCount = 0;

	// CODE
	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateDeviceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => Extension Count : %d.\n", __func__, deviceExtensionCount);

		fprintf(gpFile, "%s => FIRST CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateDeviceExtensionProperties() IS SUCCEEDED.\n", __func__);
	}

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);

	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateDeviceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s => SECOND CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s => vkEnumerateDeviceExtensionProperties() IS SUCCEEDED.\n", __func__);
	}

	char **deviceExtensionNames_Array = NULL;
	deviceExtensionNames_Array = (char **)malloc(sizeof(char *) * deviceExtensionCount);

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		deviceExtensionNames_Array[i] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(deviceExtensionNames_Array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "%s => Vulakn Device Extension Name = %s.\n", __func__, deviceExtensionNames_Array[i]);
	}

	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}
	VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (strcmp(deviceExtensionNames_Array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapChainExtensionFound = VK_TRUE;
			enabledDeviceExtensionNames_Array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	// FREE ALLOCATED MEMORY
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (deviceExtensionNames_Array[i])
		{
			free(deviceExtensionNames_Array[i]);
			deviceExtensionNames_Array[i] = NULL;
		}
	}

	if (deviceExtensionNames_Array)
	{
		free(deviceExtensionNames_Array);
		deviceExtensionNames_Array = NULL;
	}

	// DISPLAY SUPPORTED EXTENSION NAME.
	if (vulkanSwapChainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s => VK_KHR_SWAPCHAIN_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => VK_KHR_SWAPCHAIN_EXTENSION_NAME FOUND.\n", __func__);
	}

	fprintf(gpFile, "===========================================================================================\n");
	// Print only supported extension names.
	for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
	{
		fprintf(gpFile, "%s => Enabled Vulkan Device Extension NAMES = %s.\n", __func__, enabledDeviceExtensionNames_Array[i]);
	}

	return (vkResult);
}

VkResult createVulkanDevice(void)
{
	// FUNCTION DECLARATIONS
	VkResult fillDeviceExtensionNames(void);

	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// FILL DEVICE EXTENSIONS
	vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => fillDeviceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "%s => fillDeviceExtensionNames() SUCCEEDED\n", __func__);

	float pQueuePriorities[1];
	pQueuePriorities[0] = 0.0f;

	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.pQueuePriorities = pQueuePriorities;

	// INITIALIZE VkCreateDeviceInfo Structure
	VkDeviceCreateInfo vkDeviceCreateInfo;
	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_Array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;

	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;

	vkResult = vkCreateDevice(vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &vkDevice);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateDevice() Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "%s => vkCreateDevice() SUCCEEDED\n", __func__);

	return vkResult;
}

void getDeviceQueue(void)
{
	// CODE
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 0, &vkQueue);
	if (vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "%s => vkGetDeviceQueue() returned NULL for vkQuueue %d\n", __func__, graphicsQueueFamilyIndex_selected);
		return;
	}
	else
	{
		fprintf(gpFile, "%s => vkGetDeviceQueue() SUCCEEDED\n", __func__);
	}
}

VkResult getPhysicalDeviceSurfaceFormateAndColorSpace(void)
{
	// Variable Declarations
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// GET COUNT OF SUPPORTED FORMATS
	uint32_t format_count = 0;

	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &format_count, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceFormatsKHR() First Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceFormatsKHRF() First Call SUCCEEDED\n", __func__);
	}

	// DECLARE AND ALLOCATE VkSurfaceFormatKHR
	VkSurfaceFormatKHR *vkSurfaceFormatKHR_Array = (VkSurfaceFormatKHR *)malloc(format_count * sizeof(VkSurfaceFormatKHR));

	// FILLING ARRAY
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &format_count, vkSurfaceFormatKHR_Array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceFormatsKHRF() Second Call SUCCEEDED\n", __func__);
	}

	// DECIDE THE SURFACE FORMAT
	if (format_count == 1 && vkSurfaceFormatKHR_Array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_Array[0].format;
	}

	// DECIDE THE COLOR SPACE
	vkColorSpaceKHR = vkSurfaceFormatKHR_Array[0].colorSpace;

	if (vkSurfaceFormatKHR_Array)
	{
		free(vkSurfaceFormatKHR_Array);
		vkSurfaceFormatKHR_Array = NULL;

		fprintf(gpFile, "%s => vkSurfaceFormatKHR_Array free succesfully...\n", __func__);
	}

	return vkResult;
}

VkResult getPhysicalDevicePresentMode(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	uint32_t presentModeCount = 0;

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHR() First Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHRF() First Call Failed\n", __func__);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHRF() First Call SUCCEEDED\n", __func__);
	}

	// DECLARE AND ALOCATE
	VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHRF() Second Call Failed\n", __func__);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfacePresentModesKHRF() Second Call SUCCEEDED\n", __func__);
	}

	// DECIDE PRESENTATION MODE
	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			fprintf(gpFile, "%s => VK_PRESENT_MODE_MAILBOX_KHR\n", __func__);
			break;
		}
	}

	if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
		fprintf(gpFile, "%s => VK_PRESENT_MODE_FIFO_KHR\n", __func__);
	}

	if (vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;

		fprintf(gpFile, "%s => vkPresentModeKHR_array free succesfully...\n", __func__);
	}

	return vkResult;
}

///////////////////////////////////////////////////////////////////////
VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	// Variable Declarations
	VkResult vkResult = VK_SUCCESS;

	// Code

	//* Step - 1
	uint32_t formatCount = 0;
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
	else if (formatCount == 0)
	{
		fprintf(gpFile, "%s => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Returned 0 Devices !!!\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
		fprintf(gpFile, "%s => Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() SUCCEEDED\n", __func__);

	//* Step - 2
	VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	if (vkSurfaceFormatKHR_array == NULL)
	{
		fprintf(gpFile, "%s => malloc() Failed For vkSurfaceFormatKHR_array !!!\n", __func__);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	//* Step - 3
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s => Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() SUCCEEDED\n", __func__);

	//* Step - 4
	if (formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_array[0].format;
		vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
	}

	//* Step - 5
	if (vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
		fprintf(gpFile, "%s => free() SUCCEEDED For vkSurfaceFormatKHR_array\n", __func__);
	}

	return vkResult;
}

VkResult createSwapchain(VkBool32 vsync)
{
	// Function Declarations
	VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult getPhysicalDevicePresentMode(void);

	// Variable Declarations
	VkResult vkResult = VK_SUCCESS;

	//* Step - 1
	vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s => getPhysicalDeviceSurfaceFormatAndColorSpace() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s => getPhysicalDeviceSurfaceFormatAndColorSpace() SUCCEEDED\n", __func__);

	//* Step - 2
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s => vkGetPhysicalDeviceSurfaceCapabilitiesKHR() SUCCEEDED\n", __func__);

	//* Step - 3
	uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumberOfSwapchainImages = 0;

	if (vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
	else
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;

	//* Step - 4
	memset((void *)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
	if (vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{
		vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gpFile, "%s => [If Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
	}
	else
	{
		// If surface size is already defined, then swapchain image size must match with it
		VkExtent2D vkExtent2D;
		memset((void *)&vkExtent2D, 0, sizeof(VkExtent2D));

		vkExtent2D.width = (uint32_t)winWidth;
		vkExtent2D.height = (uint32_t)winHeight;

		vkExtent2D_swapchain.width = max(
			vkSurfaceCapabilitiesKHR.minImageExtent.width,
			min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));

		vkExtent2D_swapchain.height = max(
			vkSurfaceCapabilitiesKHR.minImageExtent.height,
			min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));

		fprintf(gpFile, "%s => [Else Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
	}

	//* Step - 5
	//! VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT => Mandatory
	//! VK_IMAGE_USAGE_TRANSFER_SRC_BIT => Optional (Useful for Texture, FBO, Compute)
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	//* Step - 6
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
	if (vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) //* Check For Identity Matrix
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;

	//* Step - 7
	vkResult = getPhysicalDevicePresentMode();
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s => getPhysicalDevicePresentMode() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s => getPhysicalDevicePresentMode() SUCCEEDED\n", __func__);

	//* Step - 8
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void *)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));
	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;

	vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

	vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapChainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateSwapchainKHR() Failed : %d !!!\n", __func__, vkResult);
		return vkResult;
	}
	else
		fprintf(gpFile, "%s => vkCreateSwapchainKHR() SUCCEEDED\n", __func__);

	return VK_SUCCESS;
}
///////////////////////////////////////////////////////////////////////

VkResult createImagesAndImageViews(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// GET SWAP CHAIN IMAGE COUNT
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapChainKHR, &swapChainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetSwapchainImagesKHR() IS FAILED.[1st Call]\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkGetSwapchainImagesKHR() IS SUCCEEDED.[1st Call]\n", __func__);
		fprintf(gpFile, "%s => createImagesAndImageViews() gave swapChainImageCount = %d .\n", __func__, swapChainImageCount);
	}

	// ALLOACTE THE SWAP CHAIN IMAGE ARRAY
	swapChainImage_Array = (VkImage *)malloc(sizeof(VkImage) * swapChainImageCount);

	// FILL THIS ARRAY BY SWAP CHAIN IMAGES
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapChainKHR, &swapChainImageCount, swapChainImage_Array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkGetSwapchainImagesKHR() IS FAILED.[2nd Call]\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkGetSwapchainImagesKHR() IS SUCCEEDED.[2nd Call]\n", __func__);
	}

	// ALLOACTE THE SWAP CHAIN IMAGE VIEW ARRAY
	vkSwapChainImageViews_Array = (VkImageView *)malloc(sizeof(VkImageView) * swapChainImageCount);

	// INITIALIZE VkImageViewCreateInfo Struct
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void *)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = vkFormat_color;
	// VkComponentMapping
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R; // enum VkComponentSwizzle
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	// VkImageSubresourceRange
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // VkImageAspectFlagBits
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

	// FILL IMAGE VIEW ARRAY BY USING ABOVE STRUCT
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkImageViewCreateInfo.image = swapChainImage_Array[i];
		vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkSwapChainImageViews_Array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkCreateImageView() IS FAILED FOR INTERATION %d = %d.\n", __func__, i, vkResult);
			return (vkResult);
		}
		else
		{
			fprintf(gpFile, "%s => vkCreateImageView() IS SUCCEEDED FOR INTERATION %d.\n", __func__, i);
		}
	}

	return vkResult;
}

VkResult createCommandPool(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void *)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));

	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // THIS FLAG INDIACTE THAT CREATE SUCH COMMAND POOL WHICH WILL CONTAIN SUCH COMMAND BUFFER WHICH CAN RESET AND RESTARTED.
	vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;

	vkResult = vkCreateCommandPool(vkDevice, &vkCommandPoolCreateInfo, NULL, &vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateCommandPool() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateCommandPool() IS SUCCEEDED.\n", __func__);
	}
	return vkResult;
}

VkResult createCommandBuffers(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void *)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	vkCommandBuffer_Array = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * swapChainImageCount);

	// ALLOCATE COMMAND BUFFER
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_Array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkAllocateCommandBuffers() IS FAILED.\n", __func__);
			return (vkResult);
		}
		else
		{
			fprintf(gpFile, "%s => vkAllocateCommandBuffers() IS SUCCEEDED.\n", __func__);
		}
	}

	return vkResult;
}

VkResult createVertexBuffer(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;
	VertexData vertexData_stagingBuffer_position;
	float triangle_position[] = {
		0.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f};

	// CODE
	// STEP - 1

	memset((void *)&vertexData_stagingBuffer_position, 0, sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
	memset((void *)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo_stagingBuffer.pNext = NULL;
	vkBufferCreateInfo_stagingBuffer.flags = 0;
	vkBufferCreateInfo_stagingBuffer.size = sizeof(triangle_position);
	vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // CAN USE THIS BUFFER FOR CONCURRANT USE

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateBuffer() IS FAILED FOR STAGING BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateBuffer() IS SUCCEEDED FOR STAGING BUFFER.\n", __func__);
	}

	VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
	memset((void *)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_stagingBuffer_position.vkBuffer, &vkMemoryRequirements_stagingBuffer);

	VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
	memset((void *)&vkMemoryAllocateInfo_stagingBuffer, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_stagingBuffer.pNext = NULL;
	vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
	vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0; // INITAILZIE VALUE BEFORE ENTERING INTO LOOP

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
		{
			// if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) // I HAVE UPDATED WITH BELOW LINE BEACUSE ITS FAILING FOR MY GPU
			if ((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) != 0) // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT NO NEED TO MANAGE THE VULKAN CACHE MECHANISME FOR FLUSHING & MAPPING AS WE ORDER THE VULKAN FOR MAITAIN THE COHERANCY
			{
				fprintf(gpFile, "%s => Found suitable memory type at index %u  FOR STAGING BUFFER\n", __func__, i);
				vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vertexData_stagingBuffer_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkAllocateMemory() IS FAILED FOR STAGING BUFFER. \n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkAllocateMemory() IS SUCCEEDED FOR STAGING BUFFER.\n", __func__);
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_stagingBuffer_position.vkBuffer, vertexData_stagingBuffer_position.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkBindBufferMemory() IS FAILED FOR STAGING BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkBindBufferMemory() IS SUCCEEDED FOR STAGING BUFFER.\n", __func__);
	}

	void *data = NULL;
	vkResult = vkMapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, 0, vkMemoryAllocateInfo_stagingBuffer.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkMapMemory() IS FAILED FOR STAGING BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkMapMemory() IS SUCCEEDED FOR STAGING BUFFER.\n", __func__);
	}

	// ACTUAL MMIO
	memcpy(data, triangle_position, sizeof(triangle_position)); // THIS MEMORY IS INTEGRAL ALLIGNED

	// UNMAP
	vkUnmapMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory);

	// STEP - 2

	memset((void *)&vertexData_position, 0, sizeof(VertexData));

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; // VALID FLAGS ARE USED IN SCATTERED/SPARSE BUFFERS
	vkBufferCreateInfo.size = sizeof(triangle_position);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	vkBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateBuffer() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateBuffer() IS SUCCEEDED.\n", __func__);
	}

	VkMemoryRequirements vkMemoryRequirements;
	memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
	vkMemoryAllocateInfo.memoryTypeIndex = 0; // INITAILZIE VALUE BEFORE ENTERING INTO LOOP

	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			// if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) // I HAVE UPDATED WITH BELOW LINE BEACUSE ITS FAILING FOR MY GPU
			if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				fprintf(gpFile, "%s => Found suitable memory type at index %u\n", __func__, i);
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkAllocateMemory() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkAllocateMemory() IS SUCCEEDED.\n", __func__);
	}

	vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkBindBufferMemory() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkBindBufferMemory() IS SUCCEEDED.\n", __func__);
	}

	// NO NEED FOR vkMapMemory, memcpy, vkUnmapMemory FOR THIS DEVICE VISIBLE BUFFER

	// STEP - 3
	// CREATE COMMAND BUFFER
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void *)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.commandBufferCount = 1;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vkCommandBuffer = VK_NULL_HANDLE;

	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkAllocateCommandBuffers() IS FAILED FOR BUFFER COPY IN STAGING BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkAllocateCommandBuffers() IS SUCCEEDED FOR BUFFER COPY IN STAGING BUFFER.\n", __func__);
	}

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
	memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

	vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo.pNext = NULL;
	vkCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // BEACUSE OF THIS BUT NO REQUIRED TO USE vkResetCommandBuffer, vkResetCommandBuffer IS REQUIRED FOR IF YOU ARE UPDATING BUFFER FREQUERNLTLY. THIS FLAG USED IF YOU WANT TO SUBMIT ONCE

	vkResult = vkBeginCommandBuffer(vkCommandBuffer, &vkCommandBufferBeginInfo); // LIKE VAO
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkBeginCommandBuffer() IS FAILED FOR COPY BUFFER.\n", __func__);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s => vkBeginCommandBuffer() IS SUCCEEDED FOR COPY BUFFER.\n", __func__);
	}

	// STEP - 4
	VkBufferCopy vkBufferCopy;
	memset((void *)&vkBufferCopy, 0, sizeof(VkBufferCopy));

	vkBufferCopy.srcOffset = 0; // YOU CAN KEEP MULTIPLE BUFFERSIN REGIONS IN MULTIPLE REGIONS
	vkBufferCopy.dstOffset = 0;
	vkBufferCopy.size = sizeof(triangle_position);

	vkCmdCopyBuffer(vkCommandBuffer, vertexData_stagingBuffer_position.vkBuffer, vertexData_position.vkBuffer, 1, &vkBufferCopy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCmdCopyBuffer() IS FAILED FOR COPY BUFFER.\n", __func__);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s => vkCmdCopyBuffer() IS SUCCEEDED FOR COPY BUFFER.\n", __func__);
	}

	vkResult = vkEndCommandBuffer(vkCommandBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkEndCommandBuffer() IS FAILED FOR COPY BUFFER.\n", __func__);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s => vkEndCommandBuffer() IS SUCCEEDED FOR COPY BUFFER.\n", __func__);
	}

	// STEP - 5
	// SUBMIT ABOVE COMAND BUFFER TO QUEUE
	VkSubmitInfo vkSubmitInfo;
	memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer;

	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkQueueSubmit() IS FAILED FOR COPY BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkQueueSubmit() IS SUCCEEDED FOR COPY BUFFER.\n", __func__);
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkQueueWaitIdle() IS FAILED FOR COPY BUFFER.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkQueueWaitIdle() IS SUCCEEDED FOR COPY BUFFER.\n", __func__);
	}

	// FREE STAGING COMMAND BUFFER
	if (vkCommandBuffer)
	{
		vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer);
		vkCommandBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vkCommandBuffer DESTROYED SUCCESSULLY.\n", __func__);
	}

	// STEP - 6

	// DESTROY DEVICE MEMORY
	if (vertexData_stagingBuffer_position.vkDeviceMemory)
	{
		vkFreeMemory(vkDevice, vertexData_stagingBuffer_position.vkDeviceMemory, NULL);
		vertexData_stagingBuffer_position.vkDeviceMemory = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vertexData_stagingBuffer_position.vkDeviceMemory DESTROYED SUCCESSULLY.\n", __func__);
	}

	// DESTROY VERTEX BUFFER
	if (vertexData_stagingBuffer_position.vkBuffer)
	{
		vkDestroyBuffer(vkDevice, vertexData_stagingBuffer_position.vkBuffer, NULL);
		vertexData_stagingBuffer_position.vkBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "%s => vertexData_stagingBuffer_position.vkBuffer DESTROYED SUCCESSULLY.\n", __func__);
	}

	return vkResult;
}

VkResult createShader(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// VERTEX SHADER
	const char *szFileName = "shader.vert.spv";
	FILE *fp = NULL;
	size_t size;

	fp = fopen(szFileName, "rb");
	if (fopen == NULL)
	{
		fprintf(gpFile, "%s => fopen() IS FAILED TO OPEN %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => fopen() IS SUCCEEDED TO OPEN %s.\n", __func__, szFileName);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "%s => ftell() RETURNED SIZE IS ZERO FOR %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	fseek(fp, 0L, SEEK_SET);

	char *shaderData = (char *)malloc(size * sizeof(char));

	size_t rat_val = fread(shaderData, size, 1, fp);
	if (rat_val != 1)
	{
		fprintf(gpFile, "%s => fread() IS FAILED TO READ %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => fread() IS SUCCEEDED TO READ %s.\n", __func__, szFileName);
	}

	fclose(fp);

	VkShaderModuleCreateInfo vKShaderModuleCreateInfo;
	memset((void *)&vKShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vKShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vKShaderModuleCreateInfo.pNext = NULL;
	vKShaderModuleCreateInfo.flags = 0; // RESERVEDm HENCE IT SHOULD BE 0
	vKShaderModuleCreateInfo.codeSize = size;
	vKShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vKShaderModuleCreateInfo, NULL, &vkShaderModule_Vertex);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateShaderModule() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateShaderModule() IS SUCCEEDED.\n", __func__);
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gpFile, "VERTEX SHADER MODULE IS SUCCEEDED.\n");
	fprintf(gpFile, "===========================================================================================\n");

	// FRAGMENT SHADER
	szFileName = "shader.frag.spv";
	fp = NULL;
	size = 0;

	fp = fopen(szFileName, "rb");
	if (fopen == NULL)
	{
		fprintf(gpFile, "%s => fopen() IS FAILED TO OPEN %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => fopen() IS SUCCEEDED TO OPEN %s.\n", __func__, szFileName);
	}

	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	if (size == 0)
	{
		fprintf(gpFile, "%s => ftell() RETURNED SIZE IS ZERO FOR %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}

	fseek(fp, 0L, SEEK_SET);

	shaderData = (char *)malloc(size * sizeof(char));

	rat_val = fread(shaderData, size, 1, fp);
	if (rat_val != 1)
	{
		fprintf(gpFile, "%s => fread() IS FAILED TO READ %s.\n", __func__, szFileName);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => fread() IS SUCCEEDED TO READ %s.\n", __func__, szFileName);
	}

	fclose(fp);

	memset((void *)&vKShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vKShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vKShaderModuleCreateInfo.pNext = NULL;
	vKShaderModuleCreateInfo.flags = 0; // RESERVED, HENCE IT SHOULD BE 0
	vKShaderModuleCreateInfo.codeSize = size;
	vKShaderModuleCreateInfo.pCode = (uint32_t *)shaderData;

	vkResult = vkCreateShaderModule(vkDevice, &vKShaderModuleCreateInfo, NULL, &vkShaderModule_Fragment);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateShaderModule() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateShaderModule() IS SUCCEEDED.\n", __func__);
	}

	if (shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}

	fprintf(gpFile, "FRAGMENT SHADER MODULE IS SUCCEEDED.\n");
	fprintf(gpFile, "===========================================================================================\n");

	return (vkResult);
}

VkResult createDescriptorSetLayout(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset(&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0; // reserved

	vkDescriptorSetLayoutCreateInfo.bindingCount = 0;

	vkDescriptorSetLayoutCreateInfo.pBindings = NULL;

	vkResult = vkCreateDescriptorSetLayout(vkDevice,
										   &vkDescriptorSetLayoutCreateInfo,
										   NULL,
										   &vkDescriptorSetLayout);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateDescriptorSetLayout() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateDescriptorSetLayout() IS SUCCEEDED.\n", __func__);
	}

	return vkResult;
}

VkResult createPipelineLayout(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE

	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset(&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0;

	vkPipelineLayoutCreateInfo.setLayoutCount = 1;
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

	vkResult = vkCreatePipelineLayout(vkDevice,
									  &vkPipelineLayoutCreateInfo,
									  NULL,
									  &vkPipelineLayout);

	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreatePipelineLayout() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreatePipelineLayout() IS SUCCEEDED.\n", __func__);
	}

	return vkResult;
}

VkResult createRenderPass(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkAttachmentDescription vkAttachmentDescription_Array[1];
	memset((void *)vkAttachmentDescription_Array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_Array));

	vkAttachmentDescription_Array[0].flags = 0; // VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT (Used for embeded, ekch memory muliple attachment sathhi vaprali jate)
	vkAttachmentDescription_Array[0].format = vkFormat_color;
	vkAttachmentDescription_Array[0].samples = VK_SAMPLE_COUNT_1_BIT;
	vkAttachmentDescription_Array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;	 // renderpass madhe ala ki clear kar
	vkAttachmentDescription_Array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 2 options store or dont care
	vkAttachmentDescription_Array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDescription_Array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_Array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_Array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference vkAttachmentReference;
	memset((void *)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

	vkAttachmentReference.attachment = 0;									 // varil array la 0th index ahe mhnun 0
	vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // layout asa thev hi attchment as color attachment mhnun vapar kar

	VkSubpassDescription vkSubpassDescription;
	memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // PIPELINE RELATED VARIABLE
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_Array);
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = NULL;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void *)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_Array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_Array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
	vkRenderPassCreateInfo.dependencyCount = 0;
	vkRenderPassCreateInfo.pDependencies = NULL;

	vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateRenderPass() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateRenderPass() IS SUCCEEDED.\n", __func__);
	}

	return vkResult;
}

VkResult createPipeline(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// VERTEX INPUT STATE
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
	memset((void *)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

	vkVertexInputBindingDescription_array[0].binding = 0;							  // SIMILAR TO GL_ARRAY_BUFFER
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;			  // x,y,z
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //

	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1]; // RELATED TO SHADER ATTRIBUTE
	memset((void *)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	vkVertexInputAttributeDescription_array[0].binding = 0;
	vkVertexInputAttributeDescription_array[0].location = 0;						// layout(location = 0) (shader line)
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; // SIGNED FLOAT, 32 BIT
	vkVertexInputAttributeDescription_array[0].offset = 0;							// USED FOR INTERLEAVED

	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;

	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void *)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));

	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// RISTERRISE STATE
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void *)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;

	// COLOR BLEND STATE
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void *)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void *)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;

	// VIEW PORT & SCISSOR STATE
	memset((void *)&vkViewport, 0, sizeof(VkViewport));

	vkViewport.x = 0;
	vkViewport.y = 0;
	vkViewport.width = (float)vkExtent2D_swapchain.width;
	vkViewport.height = (float)vkExtent2D_swapchain.height;
	vkViewport.minDepth = 0.0f;
	vkViewport.maxDepth = 1.0f;

	memset((void *)&vkRect2D_Scissor, 0, sizeof(VkRect2D));

	vkRect2D_Scissor.offset.x = 0;
	vkRect2D_Scissor.offset.y = 0;
	vkRect2D_Scissor.extent.width = vkExtent2D_swapchain.width;
	vkRect2D_Scissor.extent.height = vkExtent2D_swapchain.height;

	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void *)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	vkPipelineViewportStateCreateInfo.viewportCount = 1; // WE CAN SPECIFY MULTIPLE VIEWPORT
	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;

	vkPipelineViewportStateCreateInfo.scissorCount = 1;
	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_Scissor;

	// DEPTH STENCIL STATE (AS WE DONT HAVE DEPTH YET, SO WE ARE OMMITING THI STATE)

	// DYAMIC STATE (WE DONT HAVE DYANMIC STATE)

	// MULTI SAMPLING STATE
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void *)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0;
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// SHADER STAGE
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void *)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

	// VERTEX SHADER
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_Vertex;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main";
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL; // IF CONSTANTS ARE IN SHADER, SO SHADER COMPILER WILL PRECOMPILE IT, THIS TYPE SHADER IS SPECIALSED & CONSTANTS ARE SPECIALIZED.

	// FRAGMENT SHADER
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_Fragment;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main";
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL; // IF CONSTANTS ARE IN SHADER, SO SHADER COMPILER WILL PRECOMPILE IT, THIS TYPE SHADER IS SPECIALSED & CONSTANTS ARE SPECIALIZED.

	// TESSELATION STATE ( WE DONT HAVE TESSELATION STATE, SO WE ARE OMIITIN THIS STATE)

	// AS PIPELINE CREATES FROM PIPELINE CACHE, SO WE CREATING PIPELINE CACHE OBJECT
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void *)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreatePipelineCache() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreatePipelineCache() IS SUCCEEDED.\n", __func__);
	}

	// CREATE THE ACTUAL GRAPHICS PIPELINE
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void *)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));

	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
	vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL;
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
	vkGraphicsPipelineCreateInfo.subpass = 0;
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;

	// CREATE THE PIPELINE
	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateGraphicsPipelines() IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateGraphicsPipelines() IS SUCCEEDED.\n", __func__);
	}

	if (vkPipelineCache)
	{
		vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
	}

	return vkResult;
}

VkResult createFrameBuffer(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkImageView vkImageView_Attachments_Array[1];
	memset((void *)vkImageView_Attachments_Array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_Attachments_Array));

	VkFramebufferCreateInfo vkFramebufferCreateInfo;
	memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

	vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	vkFramebufferCreateInfo.pNext = NULL;
	vkFramebufferCreateInfo.flags = 0;
	vkFramebufferCreateInfo.renderPass = vkRenderPass;
	vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_Attachments_Array);
	vkFramebufferCreateInfo.pAttachments = vkImageView_Attachments_Array;
	vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
	vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
	vkFramebufferCreateInfo.layers = 1;

	vkFramebuffer_Array = (VkFramebuffer *)malloc(sizeof(VkFramebuffer) * swapChainImageCount);

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkImageView_Attachments_Array[0] = vkSwapChainImageViews_Array[i];

		vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_Array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkCreateFramebuffer() IS FAILED FOR INDEX %d.\n", __func__, i);
			return (vkResult);
		}
		else
		{
			fprintf(gpFile, "%s => vkCreateFramebuffer() IS SUCCEEDED FOR INDEX %d.\n", __func__, i);
		}
	}

	return vkResult;
}

VkResult createSemaphore(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;

	memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext = NULL;
	vkSemaphoreCreateInfo.flags = 0; // MUST BE 0, RESERVED

	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_BackBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateSemaphore() IS FAILED FOR BackBuffer.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateSemaphore() IS SUCCEEDED FOR BackBuffer.\n", __func__);
	}

	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_RenderComplete);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s => vkCreateSemaphore() IS FAILED FOR RenderComplete.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s => vkCreateSemaphore() IS SUCCEEDED FOR RenderComplete.\n", __func__);
	}

	return vkResult;
}

VkResult createFences(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	VkFenceCreateInfo vkFenceCreateInfo;

	memset(&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	vkFence_Array = (VkFence *)malloc(sizeof(VkFence) * swapChainImageCount);

	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_Array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkCreateFence() IS FAILED FOR INDEX %d.\n", __func__, i);
		}
		else
		{
			fprintf(gpFile, "%s => vkCreateFence() IS SUCCEEDED FOR INDEX %d.\n", __func__, i);
		}
	}

	return vkResult;
}

VkResult buildCommandBuffers(void)
{
	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// LOOP PER SWAPCHAIN IMAGE
	for (uint32_t i = 0; i < swapChainImageCount; i++)
	{
		// RESET COMMAND BUFFERS
		vkResult = vkResetCommandBuffer(vkCommandBuffer_Array[i], 0); // 0 DONT RELEASE THE RESOUCES WHICH IS CREATED BY THIS COMMAND POOL
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkResetCommandBuffer() IS FAILED FOR INDEX %d.\n", __func__, i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "%s => vkResetCommandBuffer() IS SUCCEEDED FOR INDEX %d.\n", __func__, i);
		}

		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;

		memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.flags = 0; // '0' for use only primary command buffer, we are not going to use simulanoously nito multiple threads
		vkCommandBufferBeginInfo.pNext = NULL;

		vkResult = vkBeginCommandBuffer(vkCommandBuffer_Array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkBeginCommandBuffer() IS FAILED FOR INDEX %d.\n", __func__, i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "%s => vkBeginCommandBuffer() IS SUCCEEDED FOR INDEX %d.\n", __func__, i);
		}

		VkClearValue vkClearValue_Array[1];

		memset((void *)vkClearValue_Array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_Array));

		vkClearValue_Array[0].color = vkClearColorValue;

		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = vkRenderPass;
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_Array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_Array;
		vkRenderPassBeginInfo.framebuffer = vkFramebuffer_Array[i];

		// BEGIN THE RENDER PASS
		vkCmdBeginRenderPass(vkCommandBuffer_Array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		{
			// BIND WITH THE PIPELINE
			vkCmdBindPipeline(vkCommandBuffer_Array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

			// BIND WITH VERTEX BUFFER
			VkDeviceSize vkDeviceSize_offset_array[1];
			memset((void *)vkDeviceSize_offset_array, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_array));

			vkCmdBindVertexBuffers(vkCommandBuffer_Array[i], 0, 1, &vertexData_position.vkBuffer, vkDeviceSize_offset_array);

			// HERE WE SHOULD CALL VULKAN DRAWING FUNCTION
			vkCmdDraw(vkCommandBuffer_Array[i], 3, 1, 0, 0);
		}
		vkCmdEndRenderPass(vkCommandBuffer_Array[i]);

		vkResult = vkEndCommandBuffer(vkCommandBuffer_Array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gpFile, "%s => vkEndCommandBuffer() IS FAILED FOR INDEX %d.\n", __func__, i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "%s => vkEndCommandBuffer() IS SUCCEEDED FOR INDEX %d.\n", __func__, i);
		}
	}

	return vkResult;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
												   VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
												   uint64_t object,
												   size_t location,
												   int32_t messageCode,
												   const char *pLayerPrefix,
												   const char *pMessage,
												   void *pUserData)
{
	// CODE
	fprintf(gpFile, "%s => PRJ_VALIDATION:%s, (%d) = %s \n", __func__, pLayerPrefix, messageCode, pMessage);

	return VK_FALSE;
}
