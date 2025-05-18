#
#  **22-VertexBuffer**
#

### STEP - 1
Declare a structure holding Vertex Buffer related two things VkBuffer Object & VkDeviceMemory Object
Declare a global structure variable we wil call at struct VertexData & Declare the global varible of this structure named VextexData_positon

### STEP - 2
Declare function createVertexBuffer, write its prototype, below CreateCommandBuffer & above createRenderPass and also call between the calls of this two.

### STEP - 3
Impliment this UDF function, Inside this function delclare our traingle position array

### STEP - 4
memset the our global vertexData_position

### STEP - 5
Declare and memset struct VkBufferCreateInfo it has 8 members we will use 5. out of them 2 are very imp. usage & size

### STEP - 6
Call VkCreateBuffer vulkan API in the .VkBufferMember of our global struct 

### STEP - 7
Declare and memset structure VkMemoryRecquirements and then call vkGetMemoryRecquirement API to get the memory required

### STEP - 8
To actual alocate memory we need call vkAllocateMemory but before that wee need to declare and memset VkMemoryAllocateInfo. imp memebrs of this structure are .MemoryTypeIndex & .AllocationSize for allocation size obtained from memory recquyirement 
For memory type index 
A. Start a loop with counter as VkPhysicalDeviceProperty.memoryTypeCount;++
B. Inside the loop check vkMemoryRecuirement.memoryTypeBits contains 1 or not, if yes 
C. Check vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags member contains VK_MEMORY_PROPERTY_HOST_VISIBLE_BITS, If yes.
D. Then this ith Index will be our memory type index if found break out of the loop, if not continue the loop by right shifting vkMemoryRequirement.memoryTypeBits by  1, over each i iteration.

### STEP - 9
Now call vkAllocateMemory and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of our global structure.

### STEP - 10
Now we have our required memory device handle as well vkBuffer handle
Bind this deviceMemory handle to Vulkan buffer handle by using vkBindBufferMemory() 

### STEP - 11
Declare a local void* buffer say "data" and call vkMamMemory to map our device memory object handle to this void* data this will allow us to do memory mapped io 
means write on void* data it will be get automatically written/copied on to the device memory object handle 

### STEP - 12
Now To Do actual memory mapped io call memcpy.

### STEP - 13
To comolete this memory mapped io finaly call vkUnMapMemory API 

### STEP - 14
In Unitialize first free the .VkDeviceMemory Member of our global structure using vkFree memory API and then destroy .vkBuffer member od our global structure by using vkDestroy Buffer API.


// VULAKN RECOMANDS/DEMANDS THE SMALL NUMBER NUMBER OF LARGE ALLOCATIONS AND USE THEM REPEATATIVELY FOR DIFFERENT RESOURCES..

## MMIO
(![MMIO text](https://github.com/Pratik-Jagadale/ARTR/blob/main/01-Vulkan/01-Windows/03-Triangle/22-VertexBuffer/mmio.png))

