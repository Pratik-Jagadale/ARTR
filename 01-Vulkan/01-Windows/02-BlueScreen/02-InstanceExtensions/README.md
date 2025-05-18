#
#  **INSTANCE EXTENSION : STEPS**
#

# STEP - 1
Find how many instance extension are supported by the vulkan driver of this version & keepit in local variable.

# STEP - 2
Allocate & Fill struct VkExtensionProperties array  corresponding to above count.

# STEP - 3
Fill & Display a local string array of extension name obtained from vkExtensionProperty.

# STEP - 4
As not required here onwards free the VkExtensionProperties array.

# STEP - 5
Find wether above extension name contain our required two extesion.
1. VK_KHR_SURFACE_EXTENSION_NAME
2. VK_KHR_WIN32_SURFACE_EXTENSION_NAME

accordingly set two global variables 
1. requiredExtesionCount
2. requiredExtensionNameArray

# STEP - 6
As not needed hence forth free a local string array.

# STEP - 7
Print wether our vulkan driver support required extension names.

# STEP - 8
Print only supported extension names.

