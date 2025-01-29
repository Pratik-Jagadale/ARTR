#
#  **PresentationSurface**
#

Presentation Surface : Depth Enabled Client Area

# STEPS

# STEP - 1
Declare global a variable to Presentation Surface

# STEP - 2
Declare & memset platform specific(Windows, Linux, Andorid) surfaceCreateInfo structure

# STEP - 3
Initialize perticularly its hInstance & hwnd members

# STEP - 4
Now call vkCreateWin32SurfaceKHR() to create Presentation Surface Object.
