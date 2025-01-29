cls

del VK.exe

del VK.obj

del VK.res

cl.exe /c /EHsc VK.c /I "C:\VulkanSDK\1.4.304.0\Include"

rc.exe VK.rc

link.exe VK.obj VK.res user32.lib gdi32.lib  /LIBPATH:"C:\VulkanSDK\1.4.304.0\Lib" /SUBSYSTEM:WINDOWS

VK.exe
