#
#  **16-FrameBuffers**
#

Framebuffer is set of Images in which graphics pipeline will render. that why framebuffer always reference to Render Pass.
OneFrameBuffer can be common top muliple render passes restrictipon is that those render passes should be compatible(attchment must be same). data not must be same, data can be different.
The nummer of framebuffers should be equal to to number of swapchain images.

FrameBuffer with no attachment called attachemtnless framebuffer it rarely used in occlusion query or image store.
Destroying FrameBuffer does not destroy Imgaes, you can use those images in different framebufers.
FrameBuffer futher used by command buffer, hence insure before destroying that framebuffer completed their execution.

### STEP - 1
Declare an array of VkImageView equal to number of attachments
In Our example array of one.

### STEP - 2
Declare and Initialize VkFrameBufferInfo Structure

### STEP - 3
Allocate the framebuffer my malloc array equal to size of swapchain image count

### STEP - 4
Start a loop for swapchain image count and call VkCreateFrame API to create FrameBuffers.

### STEP - 5
In Unitialize Destroy Framebuffer in a a loop for swapnchain image count.
