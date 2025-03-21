#
#  **18-FencesAndSemaphores**
#

### Fence
Fence - ha host varach syncronisation object ahe, device varil kam purn hoi parynt wait karto host side la.
Wait for execute batch of commands on host.

VkSemaphore Device varil Syncronisation Oject ahe, to device varil kam purn hoi parynt device vartice wait karto
Semaphore - On Vulkan GPU wait for inter queue opertions. (multiple quene astil tar, queqe 1 complete hoi parynt queue 2 wait karel) 
host os chi garaj nahi
Two states : singnaled(go ahead state) & Unsingnaled(wait state)
Two Types: Timeline & Binary
Timeline use karych Asel tar VkSemaphoreType declare karun tyat type specify karun pNext la tyach pointer pass karaycha.
ByDefault is Binary Semaphore

VkQueueWaitIdle is fine-grained synchronization

### STEP - 1
Additionaly declare two semaphore variables & Declare Fences Array of pointer type

### STEP - 2
Into CreateSemaphore function declare, Initliaze and memset VkSemaphoreInfo Structure

### STEP - 3
Call vkCreateSemaphore two times
remeber both will use same VkSemaphoreInfo strcture.

### STEP - 4
In CreateFences UDF declare, memset and initialize VkFenceCreateInfo Structure 

### STEP - 5
In this function allocate out global fence array to the size of swapchainImageCount

### STEP - 6
In loop call vkCreateFence to initialize our global fences array.

### STEP - 7
In uninitialize, first in a loop which swapchainImageCount ad counter destroy fence array object using vkDestroyFence API and then free fences array.

### STEP - 8
Destroy Both global semaphore objects with two seprate call vkDestroysemaphore API.
