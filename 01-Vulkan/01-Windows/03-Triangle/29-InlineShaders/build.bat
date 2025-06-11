cls

del VK.exe

del VK.obj

del VK.res

cl.exe /c /EHsc /MD /std:c++17 VK.cpp /I "C:\VulkanSDK\vulkan\Include" /I "C:\VulkanSDK\vulkan\Include\glslang"

rc.exe VK.rc

link.exe VK.obj VK.res user32.lib gdi32.lib  /LIBPATH:"C:\VulkanSDK\vulkan\Lib" /SUBSYSTEM:WINDOWS

VK.exe
