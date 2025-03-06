#
#  **Presentation Mode**
#

Syncronisation between Swapchain Image & SWI (Window System Imtegration - which is platform independent)
4 Types
- Immigiate (Good formance, visual appiarnce is not good at all time)
- MailBox (certain vele nantr mail yayala hava)
- Fifo
- Fifo Relax

# STEP - 1
- Call vkGetPhysicalDeviceSurfacePresenationModesKHR first to retrive the Count of Supported Presenation Mode

# STEP - 2
- Declare & Allocate array of vkPresenationModeKHR structure

# STEP - 3
- All above function again to fill above array.

# STEP - 5
- According to content of filled array decide presenatio mode

# STEP - 6 
- Free the Array
  