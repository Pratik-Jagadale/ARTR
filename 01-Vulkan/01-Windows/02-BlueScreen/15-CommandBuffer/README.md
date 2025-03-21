#
#  **Command BUFEFR**
#

### STEP - 1
Declare and Initialize struct VkCommandBufferAllocateInfo

### STEP - 2
The Number of Command Buffers and conventionaly equal to number of swapchain images

### STEP - 3
Declare Command Buffer Array globaly and allocate it the size of swapnchain image count.

### STEP - 4
In as loop which is equal to swap chain image count, allocate each command buffer in above array by using vkAllocateCommandBuffers API.
Rember at time of allocate all buffer going to emppty later wee will record graphics or compote command into it.

### STEP - 5
In Uninitialize free each command buffer by using vkFreeBuffers API in loop of swapchian image count.
Free the Command Buffer Array.
