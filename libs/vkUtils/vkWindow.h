#ifndef VK_WINDOW_H
#define VK_WINDOW_H

#include "config.h"
#include "vexel.h"  // VEXEL: https://github.com/renelindsay/Vexel
#include "Window.h"

class vkWindow : public GWindow {
    VkInstance vkInstance = nullptr;
protected:
    VkSurfaceKHR vkSurface = nullptr;
public:
    vkWindow(const char *title="vkWindow", int width=640, int height=480);
    virtual ~vkWindow();

    VkSurfaceKHR CreateVkSurface(VkInstance instance);
    bool CanPresent(VkPhysicalDevice gpu, uint32_t queue_family) const;  // Checks if surface can present given queue type.
};

#endif
