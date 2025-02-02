# Vulkan Samples

This project contains a set of Vulkan samples, using the Window, vkUtils and Vexel libraries.  
It builds with QtCreator, Android Studio or MSVC2022.  
It runs on Windows, Linux and Android.  

The Window library creates the window, and handles Keyboard, Mouse and Touch screen input.  
The unified API simplifies cross platform development and ensures consistent behavior on all three platforms.

The Vexel library loads Vulkan functions and extensions... an alternative to vulkan_wrapper or Volk.

The vkUtils library is an abstraction layer over Vulkan, and uses VMA for memory management.


##Dependencies:  
###Linux:  
run dependencies.sh  

###Windows:  
Install python3   
cd ./libs/vkUtils/Vexel   
python3 vexel_gen.py -dg   