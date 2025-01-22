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
uint32_t enabledExtensionCount = 0;
const char *enabledInstanceExtensionNames_Array[2]; // VK_KHR_SURFACE_EXTENSION_NAME & VK_KHR_WIN32_SURFACE_EXTENSION_NAME

// VULKAN INSTANCE
VkInstance vkInstance = VK_NULL_HANDLE;

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

	// VARIABLE DECLARATIONS
	VkResult vkResult = VK_SUCCESS;

	// CODE
	vkResult = createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "%s : createVulkanInstance() IS FAILED.\n", __func__);
	}
	else
	{
		fprintf(gpFile, "%s : createVulkanInstance() IS SUCCESS.\n", __func__);
	}

	return (VK_SUCCESS);
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

	// DESTROY VULKAN INSTANCE
	if (vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
	}

	if (ghwnd)
	{
		DestroyWindow(ghwnd);
	}

	if (gpFile)
	{
		fprintf(gpFile, "%s : LOG FILE SUCCESSFULLY CLOSED.\n", __func__);
		fclose(gpFile);
		gpFile = NULL;
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
	vkInstanceCreateInfo.enabledExtensionCount = enabledExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_Array;

	// Call vkCreateInstance function to get VkInstance in a global variable and do error checking.
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "%s : vkCreateInstance() IS FAILED - INCOMPATIBLE_DRIVER %d.\n", __func__, vkResult);
		return vkResult;
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

		fprintf(gpFile, "%s : Vulakn Extension Name = %s.\n", __func__, instanceExtensionNames_Array[i]);
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
			enabledInstanceExtensionNames_Array[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_Array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			win32SurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_Array[enabledExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
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

	// Print only supported extension names.
	for (uint32_t i = 0; i < enabledExtensionCount; i++)
	{
		fprintf(gpFile, "%s : Enabled Vulkan Instance Extension NAMES = %s.\n", __func__, enabledInstanceExtensionNames_Array[i]);
	}

	return (vkResult);
}
