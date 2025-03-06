#
#  **Surface Format And Surface Color Space**
#

# STEP - 1
- Call vkGetPhysicalDeviceSurfaceFormatsKhr first to retrive the Count of Supported color formats

# STEP - 2
- Declare & Allocate of Array of vkFormatStructure corresponding to above Count
- This structure has two member VkFormat & VkColorSpaceKHR

# STEP - 3
- All the same above function again but now to fill above array

# STEP - 4
- Accroding to containt of above filled array, decide the surface color format & color space

# STEP - 4
- Free the Array
  