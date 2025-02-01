#include "CInstance.h"
#include <algorithm>    // std::find
#include <assert.h>
#include <string.h>
#include <string>


//---------------------------PickList-----------------------------
CPickList::~CPickList(){}

bool CPickList::IsPicked(const char* name) const {
    for (auto item : pick_list) { if (strcmp(name, item) == 0) return true; }
    return false;
}

int CPickList::IndexOf(const char* name) {
    repeat(Count()) { if (strcmp(name, Name(i)) == 0) return i; }
    return -1;
}

bool CPickList::Has(const char* name) { return (IndexOf(name)>=0); }

bool CPickList::Add(std::initializer_list<const char*> list) {  // Return true if all items were found.
    bool found = true;
    for (auto item : list) found &= Add(item);
    return found;
}

bool CPickList::Add(const char* name) {
    int inx = IndexOf(name);
    if (inx > -1) return Add(inx);
    LOGW("%s not found.\n", name);  // Warn if picked item was not found.
    return false;
}

bool CPickList::Add(const uint32_t inx) {       // Add indexed item to picklist.
    if (inx >= Count()) return false;            // Return false if index is out of range.
    for (const char* pickItem : pick_list)       //
        if (pickItem == Name(inx)) return true;  // Check if item was already picked
    pick_list.push_back(Name(inx));              // if not, add item to pick-list
    return true;
}

void CPickList::Remove(const char* name) {
    repeat(PickCount()) if (strcmp(name, pick_list[i]) == 0) pick_list.erase(pick_list.begin() + i);
}

void CPickList::AddAll() { repeat(Count()) Add(i); }  // Pick All items
void CPickList::Clear()   { pick_list.clear(); }        // Clear Picklist
char** CPickList::PickList()    const { return (char**)  pick_list.data(); }
uint32_t CPickList::PickCount() const { return (uint32_t)pick_list.size(); }

void CPickList::Print(const char* listName) {
    printf("%s picked: %d of %d\n", listName, PickCount(), Count());
    repeat(Count()) {
        bool picked = false;
        char* name = Name(i);
        for (char* pick : pick_list) picked |= (!strcmp(pick, name));
        if (picked) { print(eRESET, "\t%s %s\n",cTICK, name); }
        else        { print(eFAINT, "\t%s %s\n"," "  , name); }
    }
}
//----------------------------------------------------------------

//----------------------------Layers------------------------------
CLayers::CLayers() {
    VKINITCHECK(vkEnumerateInstanceLayerProperties)
    VkResult result;
    do {
        uint32_t count = 0;
        result = vkEnumerateInstanceLayerProperties(&count, nullptr);
        if (result == VK_SUCCESS && count > 0) {
            item_list.resize(count);
            result = vkEnumerateInstanceLayerProperties(&count, item_list.data());
        }
    } while (result == VK_INCOMPLETE);
    ShowVkResult(result);
}
//----------------------------------------------------------------

//--------------------------Extensions----------------------------
CExtensions::CExtensions(const char* layer_name) {
    VkResult result;
    do {
        uint32_t count = 0;
        result = vkEnumerateInstanceExtensionProperties(layer_name, &count, nullptr);                  // Get list size
        if (result == VK_SUCCESS && count > 0) {                                                    //
            item_list.resize(count);                                                                // Resize buffer
            result = vkEnumerateInstanceExtensionProperties(layer_name, &count, item_list.data());  // Fetch list
        }
    } while (result == VK_INCOMPLETE);  // If list is incomplete, try again.
    ShowVkResult(result);               // report errors
}
//----------------------------------------------------------------

//----------------------Device Extensions-------------------------
void CDeviceExtensions::Init(VkPhysicalDevice phy, const char* layer_name) {
    VkResult result;
    do {
        uint32_t count = 0;
        result = vkEnumerateDeviceExtensionProperties(phy, layer_name, &count, nullptr);                  // Get list size
        if (result == VK_SUCCESS && count > 0) {                                                       //
            item_list.resize(count);                                                                   // Resize buffer
            result = vkEnumerateDeviceExtensionProperties(phy, layer_name, &count, item_list.data());  // Fetch list
        }
    } while (result == VK_INCOMPLETE); // If list is incomplete, try again.
    VKERRCHECK(result)                 // report errors
}

void CDeviceExtensions::Print() {
#ifdef NDEBUG
CPickList::Print("Device-Extensions ");  // Dont include all those strings in the release build
#else
    std::vector<std::string> vk11_list = {  // extensions promoted to Vulkan 1.1
        "VK_KHR_16bit_storage",
        "VK_KHR_bind_memory2",
        "VK_KHR_dedicated_allocation",
        "VK_KHR_descriptor_update_template",
        "VK_KHR_device_group",
        "VK_KHR_device_group_creation",
        "VK_KHR_external_fence",
        "VK_KHR_external_fence_capabilities",
        "VK_KHR_external_memory",
        "VK_KHR_external_memory_capabilities",
        "VK_KHR_external_semaphore",
        "VK_KHR_external_semaphore_capabilities",
        "VK_KHR_get_memory_requirements2",
        "VK_KHR_get_physical_device_properties2",
        "VK_KHR_maintenance1",
        "VK_KHR_maintenance2",
        "VK_KHR_maintenance3",
        "VK_KHR_multiview",
        "VK_KHR_relaxed_block_layout",
        "VK_KHR_sampler_ycbcr_conversion",
        "VK_KHR_shader_draw_parameters",
        "VK_KHR_storage_buffer_storage_class",
        "VK_KHR_variable_pointers",
    };

    std::vector<std::string> vk12_list = { // extensions promoted to Vulkan 1.2
        "VK_KHR_8bit_storage",
        "VK_KHR_buffer_device_address",
        "VK_KHR_create_renderpass2",
        "VK_KHR_depth_stencil_resolve",
        "VK_KHR_draw_indirect_count",
        "VK_KHR_driver_properties",
        "VK_KHR_image_format_list",
        "VK_KHR_imageless_framebuffer",
        "VK_KHR_sampler_mirror_clamp_to_edge",
        "VK_KHR_separate_depth_stencil_layouts",
        "VK_KHR_shader_atomic_int64",
        "VK_KHR_shader_float16_int8",
        "VK_KHR_shader_float_controls",
        "VK_KHR_shader_subgroup_extended_types",
        "VK_KHR_spirv_1_4",
        "VK_KHR_timeline_semaphore",
        "VK_KHR_uniform_buffer_standard_layout",
        "VK_KHR_vulkan_memory_model",
        "VK_EXT_descriptor_indexing",
        "VK_EXT_host_query_reset",
        "VK_EXT_sampler_filter_minmax",
        "VK_EXT_scalar_block_layout",
        "VK_EXT_separate_stencil_usage",
        "VK_EXT_shader_viewport_index_layer",
    };

    if(VK_API_VERSION == VK_API_VERSION_1_0) printf("Target API: Vulkan 1.0\n");
    if(VK_API_VERSION == VK_API_VERSION_1_1) printf("Target API: Vulkan 1.1\n");
    if(VK_API_VERSION == VK_API_VERSION_1_2) printf("Target API: Vulkan 1.2\n");

    printf("Device-Extensions picked: %d of %d\n", PickCount(), Count());
    repeat(Count()) {
        bool picked = false;
        char* name = Name(i);
        bool vk11 = std::find(vk11_list.begin(), vk11_list.end(), name) != vk11_list.end();
        bool vk12 = std::find(vk12_list.begin(), vk12_list.end(), name) != vk12_list.end();
        const char* tick11 = (VK_API_VERSION >= VK_MAKE_VERSION(1, 1, 0)) ? cTICK : " "; //Tick if we're using Vulkan 1.1
        const char* tick12 = (VK_API_VERSION >= VK_MAKE_VERSION(1, 2, 0)) ? cTICK : " "; //Tick if we're using Vulkan 1.2

        for (char* pick : pick_list) picked |= (!strcmp(pick, name));
        const char* vk = " ";
        if (vk11)   vk = "(Vulkan 1.1)";
        if (vk12)   vk = "(Vulkan 1.2)";
        if (picked) { print(eRESET, "\t%s %-40s %s\n",cTICK, name, vk); }
        else        { print(eFAINT, "\t%s %-40s %s\n"," "  , name, vk); }
    }
#endif
}
//----------------------------------------------------------------

//---------------------------CInstance----------------------------
CInstance::CInstance(const bool enable_validation, const char* app_name, const char* engine_name) {
    //if(vkCreateInstance==0) InitVulkan();  // initialize Vulkan

    CLayers layers;
#ifdef ENABLE_VALIDATION
    if(enable_validation) layers.Add({"VK_LAYER_KHRONOS_validation"});
    layers.Print();
#endif
    CExtensions extensions;
    if (extensions.Add(VK_KHR_SURFACE_EXTENSION_NAME)) {
#ifdef VK_USE_PLATFORM_WIN32_KHR
        extensions.Add(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_ANDROID_KHR
        extensions.Add(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XCB_KHR
        extensions.Add(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_XLIB_KHR
        extensions.Add(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
        extensions.Add(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
    } else LOGE("Failed to load VK_KHR_Surface");

#ifdef ENABLE_VALIDATION
    extensions.Add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);  // in Debug mode, Enable Validation
    extensions.Print();
#endif
    assert(extensions.PickCount() >= 2);
    Create(layers, extensions, app_name, engine_name);
}

CInstance::CInstance(const CLayers& layers, const CExtensions& extensions, const char* app_name, const char* engine_name) {
    Create(layers, extensions, app_name, engine_name);
}

void CInstance::Create(const CLayers& layers, const CExtensions& extensions, const char* app_name, const char* engine_name) {
    // initialize the VkApplicationInfo structure
    VkApplicationInfo app_info  = {VK_STRUCTURE_TYPE_APPLICATION_INFO};
    app_info.pNext              = nullptr;
    app_info.pApplicationName   = app_name;
    app_info.applicationVersion = 1;
    app_info.pEngineName        = engine_name;
    app_info.engineVersion      = 1;
    app_info.apiVersion         = VK_API_VERSION;

    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info    = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    inst_info.pNext                   = nullptr;
    inst_info.flags                   = 0;
    inst_info.pApplicationInfo        = &app_info;
    inst_info.enabledExtensionCount   = extensions.PickCount();
    inst_info.ppEnabledExtensionNames = extensions.PickList();
    inst_info.enabledLayerCount       = layers.PickCount();
    inst_info.ppEnabledLayerNames     = layers.PickList();

    VKINITCHECK(vkCreateInstance)
    VKERRCHECK (vkCreateInstance(&inst_info, NULL, &instance))
    LOGI("Vulkan Instance created\n");

#ifdef ENABLE_VALIDATION
    if (extensions.IsPicked(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        DebugReport.Init(instance);  // If VK_EXT_debug_report is loaded, initialize it.
#endif
}

void CInstance::Destroy() {
#ifdef ENABLE_VALIDATION
    DebugReport.Destroy();  // Must be called BEFORE vkDestroyInstance()
#endif
    if(instance) {
        vkDestroyInstance(instance, nullptr); instance=0;
        LOGI("Vulkan Instance destroyed\n");
    }
}

void CInstance::Print() { printf("->Instance %s created.\n", (!!instance) ? "" : "NOT"); }

CInstance::~CInstance() { Destroy(); }

//----------------------------------------------------------------
