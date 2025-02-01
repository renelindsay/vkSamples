/*
*--------------------------------------------------------------------------
* CInstance creates a Vulkan vkInstance, and loads appropriate
* instance-extensions for the current platform.
* In Debug mode, CInstance also loads standard validation layers.
*
* At CInstance creation time, you can override which extensions and layers get loaded,
* by passing in your own list, or CLayers and CExtensions to the CInstance constructor.
*
*
* -------Vars defined by CMAKE:-------
*  #define VK_USE_PLATFORM_WIN32_KHR    // On Windows
*  #define VK_USE_PLATFORM_ANDROID_KHR  // On Android
*  #define VK_USE_PLATFORM_XCB_KHR      // On Linux
*
*--------------------------------------------------------------------------
*/

#ifndef CINSTANCE_H
#define CINSTANCE_H

#include "Validation.h"
#include <vector>

//using namespace std;

//#define VK_API_VERSION VK_API_VERSION_1_1
#define VK_API_VERSION VK_API_VERSION_1_2

/*
template<typename T>
struct vector: public std::vector<T> {
    using std::vector<T>::vector;
    uint32_t size(){ return static_cast<uint32_t>(std::vector<T>::size());} // gets rid of size_t
};
*/

// clang-format off
//--------------------------CPickList-----------------------------
// Used for picking items from an enumerated list.
// ( See: CLayers / CExtensions / CDeviceExtensions )
class CPickList {
  protected:
    std::vector<char*> pick_list;

  public:
    virtual char* Name(uint32_t inx) = 0;                  // Return name of indexed item
    virtual uint32_t Count() = 0;                          // Return number of enumerated items
    int IndexOf(const char* name);                         // Returns index of named item
    bool Has(const char* name);                             // Returns true if item is listed
    bool Add   (std::initializer_list<const char*> list);  // Add multiple items to picklist. eg. Add({"item1","item2"})
    bool Add   (const char* name);                         // Add named item to picklist.  Returns false if not found.
    bool Add   (const uint32_t inx);                       // Add indexed item to picklist. Returns false if out of range. (start from 0u)
    void Remove(const char* name);                         // Unpick named item.
    void AddAll();                                         // Add all items to picklist
    void Clear ();                                         // Remove all items from picklist
    bool     IsPicked(const char* name)const;              // Returns true if named item is in the picklist
    char**   PickList()const;                              // Returns picklist as an array of C string pointers (for passing to Vulkan)
    uint32_t PickCount()const;                             // Returns number of items in the picklist
    void Print(const char* listName);                      // Prints the list of items found, with ticks next to the picked ones.
    // operator vector<char*>&() const {return pickList;}
    virtual ~CPickList();
};
//----------------------------------------------------------------
//----------------------------CLayers-----------------------------
struct CLayers : public CPickList {
    std::vector<VkLayerProperties> item_list;
    CLayers();
    char* Name(uint32_t inx) { return item_list[inx].layerName; }
    uint32_t Count() { return (uint32_t)item_list.size(); }
    void     Print() { CPickList::Print("Layers"); }
};
//----------------------------------------------------------------
//--------------------------CExtensions---------------------------
struct CExtensions : public CPickList {
    std::vector<VkExtensionProperties> item_list;
    CExtensions(const char* layerName = nullptr);
    char* Name(uint32_t inx) { return item_list[inx].extensionName; }
    uint32_t Count() { return (uint32_t)item_list.size(); }
    void     Print() { CPickList::Print("Extensions"); }
};
//----------------------------------------------------------------
//----------------------Device Extensions-------------------------
struct CDeviceExtensions : public CPickList {
    std::vector<VkExtensionProperties> item_list;
    void Init(VkPhysicalDevice phy, const char* layerName = nullptr);
    char* Name(uint32_t inx) { return item_list[inx].extensionName; }
    uint32_t Count() { return (uint32_t)item_list.size(); }
    //void     Print() { CPickList::Print("Device-Extensions "); }
    void     Print();  // Adds markers for vk1.1/1.2 promoted items
};
//----------------------------------------------------------------
//---------------------------CInstance----------------------------
class CInstance {
    VkInstance instance=0;
    void Create(const CLayers& layers, const CExtensions& extensions, const char* app_name, const char* engine_name);
    void Destroy();
  public:
    CInstance(const CLayers& layers, const CExtensions& extensions, const char* app_name = "VulkanApp", const char* engine_name = "");
    CInstance(const bool enable_validation = true, const char* app_name = "VulkanApp", const char* engine_name = "");

    ~CInstance();
    // CLayers     layers;
    // CExtensions extensions;
    CDebugReport DebugReport;  // Configure debug report flags here.
    void Print();
    operator VkInstance() const { return instance; }
};
//----------------------------------------------------------------
#endif
