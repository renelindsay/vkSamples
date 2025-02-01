#include "Validation.h"
#include <string.h>  // for strlen

// TODO: breakpoint using:
//  raise(SIGINT);
//  raise(SIGTRAP);


char LAST_CALL[1024]={0};
/*
//--------------------Vulkan Dispatch Table---------------------
// WARNING: vulkan_wrapper.h must be #included BEFORE vulkan.h

#ifdef VK_NO_PROTOTYPES
struct INITVULKAN {
    INITVULKAN() {
#ifdef WIN32
        EnableVTMode();
#endif
        bool success = (InitVulkan() == 1);  // Returns true if this device supports Vulkan.
        printf("Initialize Vulkan: ");
        printf(success ? GREEN"SUCCESS\n" : RED"FAILED (Vulkan driver not found.)\n");
        printf(RESET "");
    }
} INITVULKAN;  // Run this function BEFORE main.
#endif
//-------------------------------------------------------------
*/
//-----------------------Error Checking------------------------
#if !defined(NDEBUG) || defined(ENABLE_LOGGING) || defined(ENABLE_VALIDATION)
//  In Debug mode, convert a VkResult return value to a string.
const char* VkResultStr(VkResult err) {
    switch (err) {
#define STR(r) case r: return #r
        STR(VK_SUCCESS);      // 0
        STR(VK_NOT_READY);    // 1
        STR(VK_TIMEOUT);      // 2
        STR(VK_EVENT_SET);    // 3
        STR(VK_EVENT_RESET);  // 4
        STR(VK_INCOMPLETE);   // 5

        STR(VK_ERROR_OUT_OF_HOST_MEMORY);     // -1
        STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);   // -2
        STR(VK_ERROR_INITIALIZATION_FAILED);  // -3
        STR(VK_ERROR_DEVICE_LOST);            // -4
        STR(VK_ERROR_MEMORY_MAP_FAILED);      // -5
        STR(VK_ERROR_LAYER_NOT_PRESENT);      // -6
        STR(VK_ERROR_EXTENSION_NOT_PRESENT);  // -7
        STR(VK_ERROR_FEATURE_NOT_PRESENT);    // -8
        STR(VK_ERROR_INCOMPATIBLE_DRIVER);    // -9
        STR(VK_ERROR_TOO_MANY_OBJECTS);       // -10
        STR(VK_ERROR_FORMAT_NOT_SUPPORTED);   // -11
        STR(VK_ERROR_FRAGMENTED_POOL);        // -12
        STR(VK_ERROR_UNKNOWN);                // -13

        STR(VK_ERROR_OUT_OF_POOL_MEMORY);                            // -1000069000
        STR(VK_ERROR_INVALID_EXTERNAL_HANDLE);                       // -1000072003
        STR(VK_ERROR_FRAGMENTATION);                                 // -1000161000
        STR(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);                // -1000257000

        STR(VK_ERROR_SURFACE_LOST_KHR);                              // -1000000000
        STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);                      // -1000000001
        STR(VK_SUBOPTIMAL_KHR);                                      //  1000001003
        STR(VK_ERROR_OUT_OF_DATE_KHR);                               // -1000001004
        STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);                      // -1000003001
        STR(VK_ERROR_VALIDATION_FAILED_EXT);                         // -1000011001
        STR(VK_ERROR_INVALID_SHADER_NV);                             // -1000012000

        STR(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR);                 // -1000023000
        STR(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR);        // -1000023001
        STR(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR);     // -1000023002
        STR(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR);        // -1000023003
        STR(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR);         // -1000023004
        STR(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR);           // -1000023005

        STR(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);  // -1000158000
        STR(VK_ERROR_NOT_PERMITTED_EXT);                             // -1000174001
        STR(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);           // -1000255000
        STR(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR);              // -1000299000
        STR(VK_ERROR_COMPRESSION_EXHAUSTED_EXT);                     // -1000338000

        STR(VK_THREAD_IDLE_KHR);                                     //  1000268000
        STR(VK_THREAD_DONE_KHR);                                     //  1000268001
        STR(VK_OPERATION_DEFERRED_KHR);                              //  1000268002
        STR(VK_OPERATION_NOT_DEFERRED_KHR);                          //  1000268003
        STR(VK_PIPELINE_COMPILE_REQUIRED);                           //  1000297000
        STR(VK_INCOMPATIBLE_SHADER_BINARY_EXT);                      //  1000482000
#undef STR
    default: return "UNKNOWN_RESULT";
    }
}

void ShowVkResult(VkResult err) {
    if (err > 0) LOGW("%s \n", VkResultStr(err));  // Print warning
    if (err < 0) LOGE("%s \n", VkResultStr(err));  // Print error
}
#else
void ShowVkResult(VkResult err) {}
#endif
//----------------------------------------------------------------
// clang-format off
//------------------------------------DEBUG REPORT CALLBACK-----------------------------------
#ifdef ENABLE_VALIDATION

VKAPI_ATTR VkBool32 VKAPI_CALL
DebugReportFn(VkDebugReportFlagsEXT msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject,
        size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData) {
    if(objType == VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT && msgCode <= 1) return false;  // hide "Added Callback" messages
    char buf[1024]{};
    snprintf(buf, sizeof(buf), "[%s] : %s\n", pLayerPrefix, pMsg);  // msgCode is now always 0

    const char* ignore[] {                                        // ---List of errors to ignore:---
        "UNASSIGNED-CoreValidation-DrawState-InvalidImageLayout", // False positive, when using multiple subpasses
        "VUID-vkCmdTraceRaysKHR-renderpass",                      // invalid call inside renderpass
        "VUID-vkCmdTraceRaysKHR-None-02699",                      // render target image layout must match
        "VUID-VkDescriptorImageInfo-imageLayout-00344",           // render target VK_IMAGE_LAYOUT_GENERAL
        "VUID-vkCmdTraceRaysKHR-viewType-07752",                  // Cubemap ImageView type VK_IMAGE_VIEW_TYPE_2D
    };
    for(auto& e : ignore) { if(strstr(pMsg, e)) return false; }

    if(msgFlags==VK_DEBUG_REPORT_ERROR_BIT_EXT) printf("\n%s", LAST_CALL);

    switch(msgFlags){
        case VK_DEBUG_REPORT_INFORMATION_BIT_EXT          : LOGI("%s", buf);   return false;  // 1
        case VK_DEBUG_REPORT_WARNING_BIT_EXT              : LOGW("%s", buf);   return false;  // 2
        case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT  : LOGV("%s", buf);   return false;  // 4
        case VK_DEBUG_REPORT_ERROR_BIT_EXT                : LOGE("%s\n", buf); return true;   // 8 Bail out for errors
        case VK_DEBUG_REPORT_DEBUG_BIT_EXT                : LOGD("%s", buf);   return false;  //16
        default : return false; //Don't bail out.
    }
}
//--------------------------------------------------------------------------------------------

//----------------------------------------CDebugReport----------------------------------------

void CDebugReport::Init(VkInstance inst) {
    //assert(!!inst);
    vkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT)  vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT" );
    vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(inst, "vkDestroyDebugReportCallbackEXT");

    instance = inst;
    func  = DebugReportFn;                                 // Use default debug-report function.
    flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT         |  // 1
            VK_DEBUG_REPORT_WARNING_BIT_EXT             |  // 2
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |  // 4
            VK_DEBUG_REPORT_ERROR_BIT_EXT               |  // 8
            VK_DEBUG_REPORT_DEBUG_BIT_EXT               |  //16
            0;
    Set(flags, func);
}

void CDebugReport::SetFlags   (VkDebugReportFlagsEXT flags)            { Set(flags, func); Print(); }
void CDebugReport::SetCallback(PFN_vkDebugReportCallbackEXT debugFunc) { Set(flags,debugFunc);      }

void CDebugReport::Set(VkDebugReportFlagsEXT newFlags, PFN_vkDebugReportCallbackEXT newFunc){
    if(!instance) {LOGW("Debug Report was not initialized.\n"); return;}
    if(!newFunc) newFunc = DebugReportFn;  // ensure callback is not empty
    func  = newFunc;
    flags = newFlags;

    Destroy(); // Destroy old report before creating new one
    VkDebugReportCallbackCreateInfoEXT create_info = {};
    create_info.sType                              = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    create_info.pNext                              = NULL;
    create_info.flags                              = newFlags;
    create_info.pfnCallback                        = newFunc; // Callback function to call
    create_info.pUserData                          = NULL;
    VKERRCHECK(vkCreateDebugReportCallbackEXT(instance, &create_info, NULL, &debug_report_callback));
}

void CDebugReport::Destroy() {
    if (debug_report_callback) vkDestroyDebugReportCallbackEXT(instance, debug_report_callback, NULL);
}

void CDebugReport::Print() {  // print the state of the report flags
    printf("Debug Report flags : [");
    if(flags&  1) { print(eGREEN, "INFO:1 |"); } else { print(eFAINT, "info:0 |"); }
    if(flags&  2) { print(eYELLOW,"WARN:2 |"); } else { print(eFAINT, "warn:0 |"); }
    if(flags&  4) { print(eCYAN,  "PERF:4 |"); } else { print(eFAINT, "perf:0 |"); }
    if(flags&  8) { print(eRED,   "ERROR:8|"); } else { print(eFAINT, "error:0|"); }
    if(flags& 16) { print(eBLUE,  "DEBUG:16"); } else { print(eFAINT, "debug:0" ); }
    print(eRESET,"] = %d\n",flags);
}

#else   // No Validation
void CDebugReport::SetFlags(VkDebugReportFlagsEXT flags)              { LOGW("Vulkan Validation was not enabled at compile-time.\n"); }
void CDebugReport::SetCallback(PFN_vkDebugReportCallbackEXT debugFunc){ LOGW("Vulkan Validation was not enabled at compile-time.\n"); }
#endif  // ENABLE_VALIDATION

CDebugReport::CDebugReport(): vkCreateDebugReportCallbackEXT(0),vkDestroyDebugReportCallbackEXT(0),
    debug_report_callback(0), instance(0), func(0), flags(0) {}
//--------------------------------------------------------------------------------------------
