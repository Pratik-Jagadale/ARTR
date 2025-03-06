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
