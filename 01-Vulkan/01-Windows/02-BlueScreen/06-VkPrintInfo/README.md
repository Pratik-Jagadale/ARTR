#
#  **VkPrintInfo_DeviceExtension**
#

# STEP - 1
Romove local delcartion of physical device array and physical device array and declare it globally.

# STEP - 2
Acordingly remove physical device array freeying block from if statment if vkFound == VK_TRUE & we will free this in vkPrintFunction.

# STEP - 3
declare and define printVKInfo use define function with following steps

# STEP - 3.1
Start a loop using global physical device count and inside it declare and memset vkPhysicalProperties struct variable.

# STEP - 3.2
Initialize this struct variable using vkGetPhysicalDeviceProperties()

# STEP - 3.3
Print Vulkan API version using API member version. this requires threes macros.

# STEP - 3.4
Print Device Name (deviceName member of above struct)

# STEP - 3.5
Use Device Type member of above struct in switch case block and accordingly print device type

# STEP - 3.6
Print Hexa Vendor ID of the device using vendorID meber of above struct

# STEP - 3.7
Print Hexa decimal Device ID using deviceID member of above struct
Note. for the sake of conmplimtnes we can repeate STEP-6.1 to 6.8 from get physical device 

# STEP - 3.8
But now instead of assign selected queue and selected device print, wther this device supports graphics bit, compute bit, transfer bit, using if else if block
note. similarly we also can repeaet device feature from getPhysical device function and can print all around 50+ device feature. including tesselation shader and geometry shader.

# STEP - 3.9
Free PhysicalDeviceArray
