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
const char *enabledInstanceExtensionNames_Array[2]; // VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME

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

// ENTRY POINT FUNCTION
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// FUNCTION DECLARATIONS
	VkResult initialize(void);
	void uninitialize(void);
	void display(void);
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
		fprintf(gpFile, "%s : LOG FILE IS SUCCESSFULLY CREATED.\n", __func__);
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
		fprintf(gpFile, "%s : initialize()  IS FAILED.\n", __func__);
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(gpFile, "%s : initialize() IS SUCCESS.\n", __func__);
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
			if (gpFile)
			{
				fprintf(gpFile, "%s : LOG FILE SUCCESSFULLY CLOSED.\n", __func__);
				fclose(gpFile);
				gpFile = NULL;
			}
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

	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	fprintf(gpFile, "========================================================================================\n");
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : createVulkanInstance() IS FAILED.\n", __func__);
		return (VK_ERROR_INITIALIZATION_FAILED);
	}
	else
	{
		fprintf(gpFile, "%s : createVulkanInstance() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	// CREATE VULKAN PRESENTATION SURFACE
	vkResult = getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : getSupportedSurface() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : getSupportedSurface() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	// PHYSICAL DEVICE
	vkResult = getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : getPhysicalDevice() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : getPhysicalDevice() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	// PRINT VK INFO
	vkResult = printVKInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : printVKInfo() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : printVKInfo() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	// VULKAN DEVICE
	vkResult = createVulkanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : createVulkanDevice() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : createVulkanDevice() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	//  DEVICE QUEUE
	getDeviceQueue();

	// COLOR FORMAT AND COLOR SPACE
	vkResult = getPhysicalDeviceSurfaceFormateAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : getPhysicalDeviceSurfaceFormateAndColorSpace() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : getPhysicalDeviceSurfaceFormateAndColorSpace() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	// CREATE SWAPCGHAIN
	vkResult = createSwapchain(VK_FALSE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : createSwapchain() IS FAILED.\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : createSwapchain() IS SUCCESS.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");

	return (vkResult);
}

void resize(int width, int height)
{
	// CODE
	if (height == 0)
		height = 1;
}

void display(void)
{
	// CODE
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
	if (gbFullScreen)
		ToggleFullScreen();

	// NO NEED TO DESTOY SELCTED PHYSICALD DEVICE

	// DESTROY VULKAN PRESENTATION SURFACE
	if (vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL); // IT'S NOR PLATFORM SPECIFIC
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "%s : DESTROY VULKAN PRESENTATION SURFACE.\n", __func__);
	}

	// DESTROY VULKAN INSTANCE
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "%s : DESTROY VULKAN INSTANCE.\n", __func__);
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
		fprintf(gpFile, "%s : DESTROY WINDOW.\n", __func__);
	}

	if (gpFile)
	{
		fclose(gpFile);
		gpFile = NULL;
		fprintf(gpFile, "%s : LOG FILE SUCCESSFULLY CLOSED.\n", __func__);
	}
}

////////////////////////////////////////////// VULKAN RELATED FUNCTIONS //////////////////////////////////////////////

VkResult createVulkanInstance(void)
{
	// FUNCTION DECLARTION
	VkResult fillInstanceExtensionNames(void);

	// LOCAL VARIABLES
	VkResult vkResult = VK_SUCCESS;

	// CODE
	vkResult = fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : fillInstanceExtensionNames() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : fillInstanceExtensionNames() IS SUCCESS.\n", __func__);
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

	// Call vkCreateInstance function to get VkInstance in a global variable and do error checking.
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "%s : vkCreateInstance() IS FAILED - INCOMPATIBLE_DRIVER %d.\n", __func__, vkResult);
		return (vkResult);
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "%s : vkCreateInstance() IS FAILED - EXTENSION_NOT_PRESENT %d.\n", __func__, vkResult);
		return vkResult;
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkCreateInstance() IS FAILED %d.\n", __func__, vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "%s : vkCreateInstance() IS SUCCEDED.\n", __func__);
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
		fprintf(gpFile, "%s : FIRST CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateInstanceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : Extension Count : %d.\n", __func__, instanceExtensionCount);

		fprintf(gpFile, "%s : FIRST CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateInstanceExtensionProperties() IS SUCCESS.\n", __func__);
	}

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : SECOND CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateInstanceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : SECOND CALL TO vkEnumerateInstanceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateInstanceExtensionProperties() IS SUCCESS.\n", __func__);
	}

	char **instanceExtensionNames_Array = NULL;
	instanceExtensionNames_Array = (char **)malloc(sizeof(char *) * instanceExtensionCount);

	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		instanceExtensionNames_Array[i] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(instanceExtensionNames_Array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "%s : Vulakn Instance Extension Name = %s.\n", __func__, instanceExtensionNames_Array[i]);
	}

	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 win32SurfaceExtensionFound = VK_FALSE;

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
		fprintf(gpFile, "%s : VK_KHR_SURFACE_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : VK_KHR_SURFACE_EXTENSION_NAME FOUND.\n", __func__);
	}

	if (win32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "%s : VK_KHR_WIN32_SURFACE_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : VK_KHR_WIN32_SURFACE_EXTENSION_NAME FOUND.\n", __func__);
	}
	fprintf(gpFile, "========================================================================================\n");
	// Print only supported extension names.
	for (uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
	{
		fprintf(gpFile, "%s : Enabled Vulkan Instance Extension NAMES = %s.\n", __func__, enabledInstanceExtensionNames_Array[i]);
	}

	return (vkResult);
}

VkResult getSupportedSurface(void)
{
	// VARIABLE DECLARTIONS
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
		fprintf(gpFile, "%s : vkCreateWin32SurfaceKHR()  IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : vkCreateWin32SurfaceKHR() IS SUCCESS.\n", __func__);
	}
	return (vkResult);
}

VkResult getPhysicalDevice(void)
{
	// VARIABLE DECLARTIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkEnumeratePhysicalDevices() : 1 IS FAILED.\n", __func__);
		return (vkResult);
	}
	else if (physicalDeviceCount == 0)
	{
		fprintf(gpFile, "%s : physicalDeviceCount IS ZERO.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : vkEnumeratePhysicalDevices() : 1 IS SUCCESS.\n", __func__);
	}

	// Allocate VkQueueFamilyProperties Array according to above count.
	vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkEnumeratePhysicalDevices() : 2 IS FAILED.\n", __func__);
		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : vkEnumeratePhysicalDevices() : 2 IS SUCCESS.\n", __func__);
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

			fprintf(gpFile, "%s : SUCCEDED TO FREE isQueueSurfaceSupported_array.\n", __func__);
		}

		if (vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;

			fprintf(gpFile, "%s : SUCCEDED TO FREE vkQueueFamilyProperties_array.\n", __func__);
		}

		if (bFound == VK_TRUE)
			break;
	}

	if (bFound == VK_TRUE)
	{
		fprintf(gpFile, "%s : getPhysicalDevice IS SUCCEDED TO GET PHYSICAL DEVICE.\n", __func__);
	}
	else
	{
		if (vkPhysicalDevice_array)
		{
			free(vkPhysicalDevice_array);
			vkPhysicalDevice_array = NULL;
		}

		fprintf(gpFile, "%s : getPhysicalDevice IS FAIELD TO GET PHYSICAL DEVICE.\n", __func__);
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
		fprintf(gpFile, "%s : SELECTED DEVICE SUPPORTS TESSELATION SHADER.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : SELECTED DEVICE NOT SUPPORTS TESSELATION SHADER.\n", __func__);
	}

	if (vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "%s : SELECTED DEVICE SUPPORTS GEOMETRY SHADER.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : SELECTED DEVICE NOT SUPPORTS GEOMETRY SHADER.\n", __func__);
	}

	return (vkResult);
}

VkResult printVKInfo(void)
{
	// VARIABLE DECLARTIONS

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
		fprintf(gpFile, "%s : vkPhysicalDevice_array DESTROYED.\n", __func__);
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
	}

	return VK_SUCCESS;
}

VkResult fillDeviceExtensionNames(void)
{
	// VARIABLE DECLARTIONS
	VkResult vkResult = VK_SUCCESS;
	uint32_t deviceExtensionCount = 0;

	// CODE
	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : FIRST CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateDeviceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : Extension Count : %d.\n", __func__, deviceExtensionCount);

		fprintf(gpFile, "%s : FIRST CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateDeviceExtensionProperties() IS SUCCESS.\n", __func__);
	}

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);

	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : SECOND CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateDeviceExtensionProperties() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : SECOND CALL TO vkEnumerateDeviceExtensionProperties().\n", __func__);
		fprintf(gpFile, "%s : vkEnumerateDeviceExtensionProperties() IS SUCCESS.\n", __func__);
	}

	char **deviceExtensionNames_Array = NULL;
	deviceExtensionNames_Array = (char **)malloc(sizeof(char *) * deviceExtensionCount);

	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		deviceExtensionNames_Array[i] = (char *)malloc(sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(deviceExtensionNames_Array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		fprintf(gpFile, "%s : Vulakn Device Extension Name = %s.\n", __func__, deviceExtensionNames_Array[i]);
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
		fprintf(gpFile, "%s : VK_KHR_SWAPCHAIN_EXTENSION_NAME NOT FOUND.\n", __func__);

		return (vkResult);
	}
	else
	{
		fprintf(gpFile, "%s : VK_KHR_SWAPCHAIN_EXTENSION_NAME FOUND.\n", __func__);
	}

	fprintf(gpFile, "========================================================================================\n");
	// Print only supported extension names.
	for (uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
	{
		fprintf(gpFile, "%s : Enabled Vulkan Device Extension NAMES = %s.\n", __func__, enabledDeviceExtensionNames_Array[i]);
	}

	return (vkResult);
}

VkResult createVulkanDevice(void)
{
	// FUNCTION DECLARTIONS
	VkResult fillDeviceExtensionNames(void);

	// VARIABLE DECLARTIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	// FILL DEVICE EXTENSIONS
	vkResult = fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : fillDeviceExtensionNames() Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "%s : fillDeviceExtensionNames() Succeeded\n", __func__);

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
		fprintf(gpFile, "%s : vkCreateDevice() Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "%s : vkCreateDevice() Succeeded\n", __func__);

	return vkResult;
}

void getDeviceQueue(void)
{
	// CODE
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_selected, 1, &vkQueue);
	if (vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "%s : vkGetDeviceQueue() returned NULL for vkQuueue %d\n", __func__, graphicsQueueFamilyIndex_selected);
		return;
	}
	else
	{
		fprintf(gpFile, "%s : vkGetDeviceQueue() Succeeded\n", __func__);
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
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceFormatsKHR() First Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceFormatsKHRF() First Call Succeeded\n", __func__);
	}

	// DECLARE AND ALLOCATE VkSurfaceFormatKHR
	VkSurfaceFormatKHR *vkSurfaceFormatKHR_Array = (VkSurfaceFormatKHR *)malloc(format_count * sizeof(VkSurfaceFormatKHR));

	// FILLING ARRAY
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &format_count, vkSurfaceFormatKHR_Array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceFormatsKHR() Second Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceFormatsKHRF() Second Call Succeeded\n", __func__);
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

		fprintf(gpFile, "%s : vkSurfaceFormatKHR_Array free succesfully...\n", __func__);
	}

	return vkResult;
}

VkResult getPhysicalDevicePresentMode(void)
{
	// VARIABLE DECLARTIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	uint32_t presentModeCount = 0;

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHR() First Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHRF() First Call Failed\n", __func__);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHRF() First Call Succeeded\n", __func__);
	}

	// DECLARE AND ALOCATE
	VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));

	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHR() Second Call Failed : %d !!!\n", __func__, vkResult);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else if (presentModeCount == 0)
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHRF() Second Call Failed\n", __func__);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfacePresentModesKHRF() Second Call Succeeded\n", __func__);
	}

	// DECIDE PRESENTATION MODE
	for (uint32_t i = 0; i < presentModeCount; i++)
	{
		if (vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			fprintf(gpFile, "%s : VK_PRESENT_MODE_MAILBOX_KHR\n", __func__);
			break;
		}
	}

	if (vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
		fprintf(gpFile, "%s : VK_PRESENT_MODE_FIFO_KHR\n", __func__);
	}

	if (vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;

		fprintf(gpFile, "%s : vkPresentModeKHR_array free succesfully...\n", __func__);
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
		fprintf(gpFile, "%s : Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
	else if (formatCount == 0)
	{
		fprintf(gpFile, "%s : Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Returned 0 Devices !!!\n", __func__);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
		fprintf(gpFile, "%s : Call 1 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

	//* Step - 2
	VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	if (vkSurfaceFormatKHR_array == NULL)
	{
		fprintf(gpFile, "%s : malloc() Failed For vkSurfaceFormatKHR_array !!!\n", __func__);
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	//* Step - 3
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s : Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s : Call 2 : vkGetPhysicalDeviceSurfaceFormatsKHR() Succeeded\n", __func__);

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
		fprintf(gpFile, "%s : free() Succeeded For vkSurfaceFormatKHR_array\n", __func__);
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
		fprintf(gpFile, "%s : getPhysicalDeviceSurfaceFormatAndColorSpace() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s : getPhysicalDeviceSurfaceFormatAndColorSpace() Succeeded\n", __func__);

	//* Step - 2
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s : vkGetPhysicalDeviceSurfaceCapabilitiesKHR() Succeeded\n", __func__);

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
		fprintf(gpFile, "%s : [If Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
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

		fprintf(gpFile, "%s : [Else Block] => Swapchain Image Width x Swapchain Image Height = %d x %d\n", __func__, vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
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
		fprintf(gpFile, "%s : getPhysicalDevicePresentMode() Failed : %d !!!\n", __func__, vkResult);
	else
		fprintf(gpFile, "%s : getPhysicalDevicePresentMode() Succeeded\n", __func__);

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
		fprintf(gpFile, "%s : vkCreateSwapchainKHR() Failed : %d !!!\n", __func__, vkResult);
		return vkResult;
	}
	else
		fprintf(gpFile, "%s : vkCreateSwapchainKHR() Succeeded\n", __func__);

	return VK_SUCCESS;
}
///////////////////////////////////////////////////////////////////////