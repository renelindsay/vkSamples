# VEXEL - Vulkan Extension Loader

## Purpose
VEXEL (Vulkan Extension Loader) is a cross-platform Vulkan extension loader.  It simplifies Vulkan initialization, extension management, and function loading, with optional support for device-specific dispatch tables.

## Features
- **Cross-Platform Support**: Works seamlessly on Windows, Linux, and Android.
- **Simplicity**: Auto-loads core and extension Vulkan function pointers.
- **Improve Performance**: Optionally generates device-specific dispatch tables to skip runtime dispatch.
- **Flexibility**: Include the extensions you need, without having to call **vkGetDeviceProcAddr()**.

## Code Generator
First, run the vexel_gen.py Python script. It downloads the latest Khronos Vulkan headers, and generates the extension loader, (vexel.cpp and vexel.h) from it.  These files can then be added to your C/C++ project, to load Vulkan and initialize its function ponters.

## Getting Started

### Prerequisites

- Python 3.6 or newer.
- A C++ compiler supporting C++11 or later.
- On Windows: pip install requests

### Basic Usage

Download the Vulkan headers, Generate the Extension Loader and list included extensions:

    ./vexel_gen.py -dgl

#### Command-Line Options

| Flag             | Description                                     |
| ---------------- | ----------------------------------------------- |
| `-d, --download` | Download the latest Vulkan headers.             |
| `-g, --generate` | Generate the Vulkan extension loader.           |
| `-c, --clean`    | Delete Vulkan headers and generated files.      |
| `-l, --list`     | List included extensions.                       |
| `-v, --verbose`  | List extensions and their associated functions. |
| `--ext`          | Specify additional extensions to include.       |


### Add Optional Extensions
The loader always includes all Vulkan Core extensions.  
By default, the loader also includes VK_KHR extensions. (Ratified by Khronos, and widely supported.)  
Other extensions are not included by default, but may be added, using the --ext flag.  
You may add individual extensions, like VK_NV_ray_tracing, or groups, like VK_EXT or VK_NV.

### Examples

    ./vexel_gen.py -d                      # download Vulkan headers
    ./vexel_gen.py -l                      # list default included extensions (Core and VK_KHR)
    ./vexel_gen.py -l --ext VK_KHR VK_EXT  # list extensions starting with VK_KHR or VK_EXT
    ./vexel_gen.py -l --ext VK             # list ALL extensions
    ./vexel_gen.py -g --ext                # generate loader, including only core extensions
    ./vexel_gen.py -g --ext VK_KHR         # generate loader, including only core and VK_KHR


## Using Vulkan Extension Loader

1. Add vexel.cpp and vexel.h to your project.
2. Include the Vulkan header in your project.
3. Make sure to #include "vexel.h" before or instead of #include "vulkan/vulkan.h"

Vexel Auto-initializes Vulkan, so Vulkan and included extensions should now work out of the box.  
However, if you prefer to call vexInitialize() manually, comment out the 'AutoRun' line near the top of vexel.cpp.

### API Reference
```c++
    // Load Vulkan function pointers for runtime dispatch by the loader. (slow)
    // Returns true if Vulkan is available, false if not.
    bool vexInitialize();

    // Load Vulkan functions with device specific pointers, to skip runtime dispatch. (fast)
    // This makes Vulkan calls slightly faster, but limits Vulkan to a single GPU.
    void vexLoadDevice(VkDevice device);

    // Load a 'vexDeviceTable' struct with device specific pointers. (fast)
    // Allows each GPU to have its own function table, to skip runtime dispatch.
    void vexLoadDeviceTable(VkDevice device, vexDeviceTable* table);
```
## References and Credits

VEXEL was inspired by the following tools:

- [**Volk**](https://github.com/zeux/volk): A modern Vulkan function loader with device-specific dispatch table support.
- [**gl3w**](https://github.com/skaslev/gl3w): A lightweight OpenGL extension loader generator.
- **vulkan_wrapper**: Previously included in the Android NDK.

These tools laid the groundwork for VEXEL, and we thank their authors for their contributions to the Vulkan ecosystem.


## Contributing

Contributions are welcome! Please submit issues or pull requests via the GitHub repository.

## License

VEXEL is licensed under the MIT License. See the `LICENSE` file for details.
