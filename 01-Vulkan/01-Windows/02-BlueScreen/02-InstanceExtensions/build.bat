cls

del VK.exe

del VK.obj

del VK.res

cl.exe /c /EHsc VK.c

rc.exe VK.rc

link.exe VK.obj VK.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

VK.exe
