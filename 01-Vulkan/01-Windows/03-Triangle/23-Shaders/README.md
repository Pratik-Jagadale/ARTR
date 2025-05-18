#
#  **23-Shaders**
#

### STEP - 1
* Write Shaders & Compile them to SPIR-V using the shader compilation tools tha we recieved in vulkan sdk.

### STEP - 2
* Global declare 2 shader object module variable of VkShaderModuleType to hold vulkan compatible Vertex Shader module object respevctively.

### STEP - 3
* Declare prototype of user defined function. createShaders() in initialize followiing convention that is after createVertexBuffer function & before createRenderPass Function.

### STEP - 4
* Using same above conevention call createShaders() between the calls of the above two.,

### STEP - 5
* Mainting the same above convention while defining the createShaders() between the definition of of two.

### STEP - 6
* Inside our function first open the shader file, set the file pointer at the end of the file, find the byte side of shader file data,reset the file pointer at the begging of ther file, allocate a character buffer of file size and read shader file data into it. & close the file. Do this things using conventional file io.

### STEP - 7
* Declare and memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.

### STEP - 8
* Call vkCreateShaderModule API, pass above structure pointer to it as parameter and obtain the shader module object in global variable. that declared the step 2;

### STEP - 9
* Free the Shader code buffer which we allocated in step 6.

### STEP - 10
* assuming we did above 4 steps. 6 to 9 for Vertex Shader. repeat them all for fragment shader too.

### STEP - 11
* In Uninitailize destroy both global shader object using vkDestroyShaderModule().

##

* Vulkan accept the spir-v format only.
* Standard for Shader Portable Intermegiate represenation Vulkan
Portbale - Accross rendering tech. (OpenGL 4.5, 4.6, DirectX, Metal, Across all rendering technology & GPU)
It's Intermegiate format. GPU will convert to gpu assembly format. spir-v is not final format.
So Nvidia will convert to nvidia asembly, amd to amd ads like that.
* 0x07230203 is Magic number of SPIR-V shader
* Command (Use PowerShell):
    `cd to Path.`
    `Format-Hex -path .\shader.vert.spv | Select-Object -First 1`
    Show the hex decimal for file. and then 2 nd command to fetch first line of output (Select-Object -First 1).

    Output: `00000000   03 02 23 07 00 00 01 00 0B 00 08 00 15 00 00 00  ..#.............`

    * Linux Command: xxd -e file.name.spv | head -n 1 (xxd == hexa decimal dump) (-e little endian, otherwise it will take cpu endianess) 
    * also you can use xxd == od (octal dump). objdump (usded for .obj files)
* SPIR-V is writes its format in SSA.(Single Static Assignment)
* Virtual Register tokens appers only once. (for better optimization)
* 

[SPIR-V Spec](https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html)

## GL_ARB_seperate_shader_objects
* Every shader will kept seprate object. 
* Unnecessory copying data of stage to stage. potenatila performnacer benifit.
* Resouces segration will very well internaly. UBO VBO etc.
* Better Debugging.

## layout(location = 0)
* Binding to location 0. 
* GL_ARB_seperate_shader_objects dermands to give location explicetly.
* Similar to glBindAttribLocation in OpenGL.

## Vulkan Shader Tools

| Tool                   | Purpose                                           | Input     | Output                                           |
| ---------------------- | ------------------------------------------------- | --------- | ------------------------------------------------ |
| `glslangValidator.exe` | Validates and compiles GLSL/HLSL to SPIR-V        | GLSL/HLSL | `.spv` binary  (Khronos official declared)       |
| `glslc.exe`            | CLI shader compiler (like `gcc` for GLSL)         | GLSL      | `.spv` binary  (we are going to use for Android) |
| `glslang.exe`          | Internal component of `glslangValidator`          | *(N/A)*   | *(Used internally)*                              |
| `spirv-dis.exe`        | Disassembles SPIR-V binary to human-readable text | `.spv`    | `.spvasm` (text)                                 |


# Command :
`C:\VulkanSDK\vulkan\Bin\glslangValidator.exe -V -H -o shader.vert.spv shader.vert`
`C:\VulkanSDK\vulkan\Bin\glslangValidator.exe -V -H -o shader.frag.spv shader.frag`

Parameters Info:
-V - for Vulkan
-H - Human readable dump
-o - Output File Name
shader.vert - Input File

.spv is non human readble. but on cmd you can see log its was shader dump.

