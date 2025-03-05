#
#  **PhysicalDevice**
#

# STEP - 1
- Declare Three Global Variables
1. Selected Physical device
2. Selected Queue Family Indexc
3. Physical Device Memory Properties(Required Later)

# STEP - 2
- Call vkEnumeratePhysicalDevices to get physical device count

# STEP - 3
- Allocate VkPhysicalDevice Array according to above count.

# STEP - 4
- Call vkEnumeratePhysicalDevices again to fill above Array.

# STEP - 5
Start a loop using above physical device count and array.
Note. Declare boolean bFound variable before this loop which will reside wether we found desired physical device or not.

# STEP - 6 INSIDE ABOVE LOOP
1.  Declare a local varaible count to hold queue count.
2.  Call vkGetPhysicalDeviceQueueFamilyProperties() to initialize above queue count.
3.  Allocate VkQueueFamilyProperties Array according to above count.
4.  Call vkGetPhysicalDeviceQueueFamilyProperties again to fill above array.
5.  Declare VkBool32 type Array & allocate it using same above Queue count.
6.  Start a Nested Loop & Fill above VkBool type array by calling vkGetPhysicalDeviceSurfaceSupportKHR
7.  Start another nested loop not nested inside in above loop. (nested in main loop). Check wether PhysicalDevice in its Array with its Queue Family has Graphics BIT or NOT if Yes then this is selected physical device assigned it to global variable Similary this index is selcted family queue index, assigned it to global variable. and set bFound equal to True and break from 2nd Nested loop.
8. Now we are back in Main Loop
9. Free the VkQueueFamilyProperties & VkBool32 Two arrays.
10. Still being in main loop. according to bFound variable break the main loop.
11. Free Physical Device Array.

# STEP - 7
Do Error Checking according value bFound.

# STEP - 8
memset the global physical device mempry property structure.

# STEP - 9
Initialized it with vkGetPhysicalDeviceMemoryProperties

# STEP - 10
Declare a local structure variable VkPhysicalDeviceFeatures, memset it & initailized it by calling vkGetPhysicalDeviceFeature

# STEP - 11
By Using Tesselation shader member of above structure check selcted devices tesselation shader support.

# STEP - 11
By Using Geometry shader member of above structure check selcted devices Geometry shader support.

# STEP - 12
There is no need ti free/destroy selected device beacuse later we will create vulkan logical device which we need to destroy & It's destruction will automatticaly destroy physical device. 
