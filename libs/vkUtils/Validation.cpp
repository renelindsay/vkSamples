#include "Validation.h"
#include <string.h>  // for strlen

char LAST_CALL[1024]={0};

#ifdef WIN32
struct INITVT{INITVT(){EnableVTMode();}}INITVT;
#endif

//-----------------------Error Checking------------------------
#if !defined(NDEBUG) || defined(ENABLE_LOGGING) || defined(ENABLE_VALIDATION)
void ShowVkResult(VkResult err) {
    if (err > 0) LOGW("%s \n", VkResultToString(err));  // Print warning
    if (err < 0) LOGE("%s \n", VkResultToString(err));  // Print error
}
#else
void ShowVkResult(VkResult err) {}
#endif
//----------------------------------------------------------------

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
