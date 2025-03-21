#
#  **19-BuildCommandBuffers**
#

### STEP - 1
Start a loop with swapchainImageCount as counter. 

### STEP - 2
Inside Loop call ckResetCommandBuffer to reset the containt of Command Buffer

### STEP - 3
then declare, memset & initiliaze vkCommandBBufferBeginInfo Structurte 

### STEP - 4
Now Call vkBeginCommandBuffer API to record Vulkan Drawing related Commands
Do Error checking

### STEP - 5
Declare Memset & Initialize struct Array of VkClearValue Type.
rember intarnaly this is union 
Our array number is 1, this number depends on number of attachment of framebuffer. as we have only one attachment color attachement hence we have of only one element.
One member array color member is meaning full whil depthctencil is meaningless, two member array color mmber is meaning less and depthstencil if meaningfull

To this color member we need to assign VkClearColorValue struct.
Todo this declare globaly VkClearColorValue structure variable & memset and initialize it in Initilaize function.
Remeber we are going to .color clear VkClearValue Structure by VkClearColor structure beacuse In Step 16 of RenderPass we specified .loadOp member of VkAttachmentDesc Structure to VK_ATTACHMENT_LOAD_OP_CLEAR

### STEP - 6
Then declare memset and initialize VkRenderPassBeginInfo structure 

### STEP - 7
BeginRenderPass by vkCmdBeginRenderPass() API
remeber code writen inside every beginRenderPass endRenderPass itself is code of subpass if no subpass is explictely created in other words even no subpass declared

### STEP - 8
End the render pass vkCmdEndRenderPass() API

### STEP - 9
End the recording of Command Buffer by vkEndCommandBuffer() API.

### STEP - 10
Close the loop
