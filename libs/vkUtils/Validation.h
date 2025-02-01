/*
*--------------------------------------------------------------------------
* The CDebugReport class is used internally by CInstance, to enable
* the validation layers to print debug/error/info messages.
*
*----------------Flags:--------------
*  #define ENABLE_VALIDATION 1          // Enables Vulkan Validation
*  #define ENABLE_LOGGING    1          // Enables LOG* print messages
*------------------------------------
*--------------------------------------------------------------------------
*/

#define USE_VULKAN_WRAPPER 1

//---Enable Validation and Logging---
#if !defined(NDEBUG)
  #define ENABLE_VALIDATION 1
  #define ENABLE_LOGGING 1
#endif
//-----------------------------------

#ifndef VALIDATION_H
#define VALIDATION_H

#include "Logging.h"
#include <stdlib.h>

#if defined(__linux__) && !defined(__ANDROID__)  // Linux (desktop only)
#define __LINUX__ 1
#endif

#undef repeat
#define repeat(COUNT) for (uint32_t i = 0; i < COUNT; ++i)

//===========================================Check VkResult=============================================
// Macro to check VkResult for errors(negative) or warnings(positive), and print as a string.
#ifdef NDEBUG                           // In release builds, don't print VkResult strings.
#define VKERRCHECK(VKFN) { (void)VKFN; }
#define VKINITCHECK(VKFN) {}
#else                                   // In debug builds, show warnings and errors. assert on error.

extern char LAST_CALL[1024];  // Location of last call made with VKERRCHECK

#define VKLASTCALL(VKFN) {                                                      \
    sprintf(LAST_CALL, "Last call: " #VKFN " :\n%s:%d\n", __FILE__, __LINE__);  \
}

#define VKERREXIT(VKFN) {                                 \
    printf("       At: %s:%d\n", __FILE__, __LINE__);     \
    printf("       Fn: " #VKFN "\n");                     \
    {abort();}                                            \
}

#define VKINITCHECK(VKFN) {                               \
    void* pPtr = (void*&) VKFN;                           \
    if(pPtr == 0) {                                       \
        LOGE("Vulkan dispatch table not initialized.\n"); \
        VKERREXIT(VKFN)                                   \
    }                                                     \
}

#define VKERRCHECK(VKFN) {                                \
    VKINITCHECK(vkCreateInstance);                        \
    VKLASTCALL(VKFN);                                     \
    VkResult VKRESULT = VKFN;                             \
    if(VKRESULT != 0) {                                   \
        ShowVkResult(VKRESULT);                           \
        if(VKRESULT<0) VKERREXIT(VKFN)                    \
    }                                                     \
    sprintf(LAST_CALL, " ");                              \
 }


#endif

//======================================================================================================

//======================================================================================================
#include <assert.h>
#include <stdio.h> //for Windows.

//=========================================== Vulkan Wrapper ===========================================
//  WARNING: If you enable USE_VULKAN_WRAPPER, make sure vulkan_wrapper.h is #included BEFORE vulkan.h
//
//#define USE_VULKAN_WRAPPER

//#ifdef USE_VULKAN_WRAPPER
//#include <vulkan_wrapper.h>  // PC: Build dispatch table
//#else
//#include <vulkan/vulkan.h>   // Android: This must be included AFTER native.h
//#endif

#include "vexel.h"
//======================================================================================================

void ShowVkResult(VkResult err);  // Print warnings and errors.

//============================================ CDebugReport ============================================
class CDebugReport {
    CDebugReport();
    PFN_vkCreateDebugReportCallbackEXT  vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    VkDebugReportCallbackEXT            debug_report_callback;
    VkInstance                          instance;
    PFN_vkDebugReportCallbackEXT        func;
    VkDebugReportFlagsEXT               flags;

    void Set(VkDebugReportFlagsEXT flags, PFN_vkDebugReportCallbackEXT debugFunc = 0);
    void Print();  // Print the debug report flags state.

    friend class CInstance;                                    // CInstance calls Init and Destroy
    void Init(VkInstance inst);                                // Initialize with default callback, and all flags enabled.
    void Destroy();                                            // Destroy the debug report. Must be called BEFORE vkDestroyInstance()

  public:
    VkDebugReportFlagsEXT GetFlags() { return flags; }         // Returns current flag settings.
    void SetFlags(VkDebugReportFlagsEXT flags);                // Select which typ/e of messages to display
    void SetCallback(PFN_vkDebugReportCallbackEXT debugFunc);  // Set a custom callback function for printing debug reports
};
//=======================================================================================================

#endif
