# vkUtils

This folder contains a set of Vulkan wrapper classes, to reduce boiler-plate code, and simplify Vulkan app development.

## Classes

## CInstance class
The CInstance class creates a VkInstance, and loads appropriate layers and platform-specific WSI Surface extensions.  
CInstance may be passed to any vulkan function that expects a VkInstance.
If the "enable_validation" constructor parameter is set (default), Standard validation layers are loaded.

Also, the following extensions are loaded where available:  
 > `VK_KHR_surface . . . . ` (On all platforms)  
 > `VK_KHR_win32_surface . ` (On Windows)  
 > `VK_KHR_xcb_surface . . ` (On Linux)  
 > `VK_KHR_android_surface ` (On Android)  
 > `VK_KHR_debug_report. . ` (When validation is enabled)   
 
If you need direct control over which layers and extensions to load, use the CLayers and CExtensions classes to enumerate, display and pick the items you want, and then pass them to the CInstance constructor.
The VkInstance is used in 2 places: Pass it to vkWindow.CreateVkSurface() to get the VkSurfaceKHR, and pass it to CPhysicalDevices, to enumerate the available GPUs.

### CLayers class
The CLayers class wraps "vkEnumerateInstanceLayerProperties" to simplify enumerating, and picking instance layers to load.  On creation, it contains a list of available instance layers, and provides functions for picking which ones to load. Here are some of the useful functions:  
 - ` Clear . :` Clear the picklist.  
 - ` Pick . .:` Add one or more named items to the picklist. eg. layers.Pick({"layer1","layer2"});  
 - ` PickAll :` Adds all available layers to the picklist.  
 - ` PickList:` Returns the picklist as an array of layer names, which can be passed to CInstance.  
 - ` Print . :` Prints the list of available layers, with a tick next to the ones what have been picked.

### CExtensions class
The CExtensions class wraps "vkEnumerateInstanceExtensionProperties" in much the same way as CLayers wraps the layers.
It provides the same functions as CLayers, for picking  extensions to load, and may also be passed to the CInstance constructor.

### Vulkan Validation Layers and Logging
vkWindow makes use of Validation Layers, via the VK_KHR_debug_report extension, to display helpful, color-coded log messages, when Vulkan is used incorrectly. (Errors / Warnings / Info / etc.)  By default, vkWindow enables standard validation layers, but they may be turned off for better runtime performance. 

The LOG functions may be used in the same way as "printf", but with some advantages:
On desktop, LOG messages are color-coded, for better readability, and on Android, they are forwarded to Android Studio's logcat facility.  Log messages can be easily stripped out, by turning off the "ENABLE_LOGGING" flag. This will reduce clutter, and keep the executable as small as possible.  
Here are some examples of LOG* message usage:

        LOGE("Error message\n");    // Errors are printed in red
        LOGW("Warning message\n");  // Warnings are printed in yellow
        LOGI("Info message\n");     // Info is printed in green

### CPhysicalDevices class
The CPhysicalDevices class wraps an array of VkPhysicalDevice objects, and is used to enumerate the available GPUs, and their properties, features and available queues.  It requires a VkInstance as input, which you can acquire from the CInstance class.
Use the FindPresentable() member function, to find which CPU can present to a given window surface (VkSurfaceKHR), and use it to create a Logical device instance. (CDevice)

### CDevice class
The CDevice class takes the chosen GPU (CPhysicalDevice) from CPhysicalDevices, and allows you to create one or more queues of specified types, using the AddQueue() function. Optionally, you can pass in a VkSurfaceKHR to this function, if you want the queue to be presentable.  Available queue types is system specific, and AddQueue() returns 0 if the current system is unable to create a queue of the specified type, in which a case you may have to fall back to an alternative queue configuration.  
eg. If AddQueue() fails to create a Presentable Graphics queue, you may have to create separate queues for graphics and presentation.

### CBuffers unit
CBuffers uses AMD's VulkanMemoryAllocator (VMA) to allocate GPU memory for various buffer types:   

* VBO : Vertex Buffer Object  (Holds an array of vertex attributes.)
* IBO : Index Buffer Object   (Holds an index array into the VBO.)
* UBO : Uniform Buffer Object (Holds a struct of shader uniform values.)
* CvkImage : for loading texture images.

### CRenderpass
Use the CRenderpass class, to configure the color/depth/stencil attachments for your render target, as well as subpasses.

### CFBO
The CFBO class (Frame Bufer Object) class holds your framebuffer and back-buffers.
Use this as your render target for offscreen rendering.  For on-screen rendering, use CSwapchain instead.

### CSwapchain
CSwapchain is derived from FBO, and adds the ability to present (display) the rendered image to a window or fulscreen surface.  You can configure your swapchain for double or tripple buffering, and select various VSync types.  Creating a CSwapchain, requires a CRenderpass, as well as valid graphics and present queues.



### CShader
Use CShader to load your vertex and fragment shaders.
CShader uses SPIR-V reflection to extract binding point names from the loaded shaders.
You may then bind VBO/IBO/UBO and Image resources to the shader variables by name.
Then use the CreatePipelineLayout() and CreateDescriptorSet() functions, to generate
appropriate VkPipelineLayout and VkDescriptorSet objects.
CShader also generates the pipeline vertex input structures, used by CPipeline.
Don't create CShader directly.  There are already 2 instances embedded as member variables
of CPipeline... one for vertex shaders, and one for fragment shaders.
 
### CPipeline
This class requires a CRenderpass and CShaders, and creates your Graphics pipeline. (WIP)

