#include "Logging.h"
#include "Validation.h"
#include "vkWindow.h"

vkWindow::vkWindow(const char *title, int width, int height) : CWindow(title, width, height) {}

vkWindow::~vkWindow() {
    if(m_vkSurface) vkDestroySurfaceKHR(vkInstance, m_vkSurface, nullptr); m_vkSurface = nullptr;
    LOGI("Vulkan Surface destroyed\n");
}

VkSurfaceKHR vkWindow::CreateVkSurface(VkInstance instance) {
    if(m_vkSurface) return m_vkSurface;
    vkInstance = instance;
#ifdef WIN32
    struct native_handle {
        HINSTANCE hInstance;
        HWND hWnd;
    };
    //native_handle* hnd =(native_handle*)m_wsiWindow.GetNativeHandle();
    native_handle* hnd =(native_handle*)GetNativeHandle();
    VkWin32SurfaceCreateInfoKHR win32_createInfo;
    win32_createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32_createInfo.pNext     = NULL;
    win32_createInfo.flags     = 0;
    win32_createInfo.hinstance = hnd->hInstance;
    win32_createInfo.hwnd      = hnd->hWnd;
    VKERRCHECK(vkCreateWin32SurfaceKHR(instance, &win32_createInfo, NULL, &m_vkSurface));
#elif __ANDROID__
    ANativeWindow* hnd = (ANativeWindow*)GetNativeHandle();
    VkAndroidSurfaceCreateInfoKHR android_createInfo {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR};
    android_createInfo.pNext  = NULL;
    android_createInfo.flags  = 0;
    android_createInfo.window = hnd;  //app->window;
    VKERRCHECK(vkCreateAndroidSurfaceKHR(instance, &android_createInfo, NULL, &m_vkSurface));
#elif __LINUX__
    struct native_handle {
        xcb_connection_t* xcb_connection;
        xcb_screen_t* xcb_screen;
        xcb_window_t xcb_window;
    };

    native_handle* hnd =(native_handle*)GetNativeHandle();
    VkXcbSurfaceCreateInfoKHR xcb_createInfo;
    xcb_createInfo.sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    xcb_createInfo.pNext      = nullptr;
    xcb_createInfo.flags      = 0;
    xcb_createInfo.connection = hnd->xcb_connection;
    xcb_createInfo.window     = hnd->xcb_window;
    VKERRCHECK(vkCreateXcbSurfaceKHR(instance, &xcb_createInfo, NULL, &m_vkSurface));
#else 
    LOGE("Unsupported platform\n");
    return 0;
#endif
    LOGI("Vulkan Surface created\n");
    return m_vkSurface;
}

bool vkWindow::CanPresent(VkPhysicalDevice gpu, uint32_t queue_family) const {
    // If surface was created, use this method
    if(m_vkSurface) {
        VkBool32 can_present = false;
        VKERRCHECK(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue_family, m_vkSurface, &can_present));
        return !!can_present;
    }

    // If no surface was created, use native handle instead
#ifdef WIN32
    return vkGetPhysicalDeviceWin32PresentationSupportKHR(gpu, queue_family) == VK_TRUE;

#elif __LINUX__
    struct native_handle {
        xcb_connection_t* xcb_connection;
        xcb_screen_t* xcb_screen;
        xcb_window_t xcb_window;
    };

    native_handle* hnd =(native_handle*)GetNativeHandle();
    return vkGetPhysicalDeviceXcbPresentationSupportKHR(gpu, queue_family, hnd->xcb_connection, hnd->xcb_screen->root_visual) == VK_TRUE;

#elif __ANDROID__
    return true;  // There's no vkGetPhysicalDeviceAndroidSupportKHR. Just assume all queue families can present
#endif
  //return false;
}

