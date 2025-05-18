#
#  **INSTANCE CREATION : STEPS**
#

# STEP - 1
As explained before fill and initialize required extension name and count in global variables

# STEP - 2
Initialize struct VkApplicationInfo

# STEP - 3
Initialize struct VkInstaceCreateInfo by using information from STEP - 1 & STEP - 2

# STEP - 4
Call vkCreateInstance function to get VkInstance in a global variable and do error checking.

# STEP - 5
Destroy VkInstance in uninitailize.

Our Extension will be enabled in vkCreateInstance()
