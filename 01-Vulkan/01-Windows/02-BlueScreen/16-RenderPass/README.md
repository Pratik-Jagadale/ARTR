#
#  **16-RenderPass**
#

All memebers of all structure neccessory in render pass.

### STEP - 1
Declare & Initialize VkAttachmentDescrition ARRAY STRUCTURE [Number of elemets in Array Defends on attachemenmt]
although in this example only one atchmet is there.[Color]

### STEP - 2
Declare and Initailize VkAttachmentReference Structure which will have information about the attachment we described above.

### STEP - 3
Declare & Initialize VkSubpassDescription Structure & keep refernce about VkAttachmentReference Structure init

### STEP - 4
Declare & Initialize VkRenderPassCreateInfo & Refer structure VkAttachmentDescrition & VkSubpassDescription into it.
Remember here also we need attachment info in the form of image views wich will used by frame buffer later.

### STEP - 5
Now call the VkCreateRenderPass function to create render pass.
Remember we also need to specify interdependency in subpasses if needed.

### STEP - 6
In Uninitialize destroy the by using VkDestroyRenderPass function.
