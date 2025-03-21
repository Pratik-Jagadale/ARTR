#
#  **SwapChain Images And Image Views**
#
Image View is container of Image

### STEP - 1
Get Swapchain Image count in global variable using vkGetSwapChainImagesKHR API.

### STEP - 2
Declare a global vkImage type array and allocate it to the swapchain image count using malloc.

### STEP - 3
Now the call the same function again and fill this array.

### STEP - 4
Declare another global array of type vkImageView and allocate it the size of swapchain image count.

### STEP - 5
Declare and Initialize VkImageViewCreateInfo struct except its ".image" member.

### STEP - 6
Now start a loop for SwapChainImage Count and inside this loop initialize above .image member to the swapchain image array index we obtained above.
And then call vkCreateImageView API to fill above ImageViewArray

### STEP - 7
In uninitialize keeping the destructor logic aside for while, first destroy swapchain images from the swapnchain images from swapnchainImages Array in loop using vkDestroyImage API.

### STEP - 8
In Unitialize now actualy free the swapchain image array using free.

### STEP - 9
In Unitialize destroy image views from image views array in loop by using vkDestroyImageViews API.

### STEP - 10
In Unitialize now acatually free the image view array.
