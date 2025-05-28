#
#  **Staging Buffer**
#

# Algoritham/Code

# STEP-1
Create a local Vertex Buffer just like previously created vertex buffer but with different usage and different sharing mode.
Identified it as stagging buffer

# STEP-2
Then Create the usual vertex buffer visible to the only device.

# STEP-3
This buffer is visible to host and can be mapped device memory.

# STEP-4
Create a command buffer

# STEP-5
Build above command buffer by vulkans buffer using buffers copy command and copy tha data fin staging buffer from step 1 to destination buffer in Step 2.

# STEP-6
while Doing this use our command queue and the command buffer to submit the work of buffer copy.

# STEP-7
After done, free/destroy the command buffer & the local staging buffer as its job is done.
