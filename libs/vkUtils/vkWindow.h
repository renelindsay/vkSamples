#ifndef VK_WINDOW_H
#define VK_WINDOW_H

#include "Validation.h"
#include "Window.h"

class vkWindow : public CWindow {
    VkInstance vkInstance = nullptr;
protected:
    VkSurfaceKHR m_vkSurface = nullptr;
public:
    vkWindow(const char *title="vkWindow", int width=640, int height=480);
    virtual ~vkWindow();

    VkSurfaceKHR CreateVkSurface(VkInstance instance);
    bool CanPresent(VkPhysicalDevice gpu, uint32_t queue_family) const;  // Checks if surface can present given queue type.
};

#endif
