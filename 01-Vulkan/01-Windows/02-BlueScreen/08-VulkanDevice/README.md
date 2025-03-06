#
#  **Creating Vulkan Logical Device**
#

# STEP - 1
- Create usee define function createVulkanDevice

# STEP - 2
- Call previously created fillExtensionDeviceNameInit Function

# STEP - 3
- Declare and initialize vkDeviceCreateInfo structure
 
# STEP - 4
- Use Previosuly obtained device exntion count and device extension array, to initialize this structure

# STEP - 5
- Call vkDeviceCreate to create the actual device & do error checking.

# STEP - 6
- Destory this device when done. 
- before destorying the device ensure the that all operation on that device are finished. '
- till that wait on device.
  
