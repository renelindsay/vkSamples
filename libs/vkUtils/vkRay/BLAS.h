#ifndef BLAS_H
#define BLAS_H

//#include "vulkan_wrapper.h"
#include "vexel.h"
#include "Buffers.h"
//#include "matrix.h"
#include "vkray_helpers.h"

//GeometryInstance
struct MeshObject {
    VkBuffer     vertexBuffer;
    uint32_t     vertexCount;
    uint32_t     vertexStride;
    VkDeviceSize vertexOffset=0;
    VkBuffer     indexBuffer;
    uint32_t     indexCount;
    VkDeviceSize indexOffset=0;
    VkBuffer     uniformBuffer;
    VkDeviceSize uniformOffset;
    bool isOpaque = true;
};


typedef std::vector<MeshObject> MeshList;
typedef std::vector<ABO>        BLASList;

//--------------------------------------------------------------------------------------------------

//---------------------------------------------QUERY------------------------------------------------
struct Query {  // query AS compacted size
    VkDevice    device     = VK_NULL_HANDLE;
    VkQueryPool query_pool = VK_NULL_HANDLE;
    Query(CCmd& cmd, VkAccelerationStructureKHR* as, VkQueryType queryType=VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR);
    ~Query();
    VkDeviceSize GetResult();
};
//--------------------------------------------------------------------------------------------------

//-------------------------------------------BlasInfo-----------------------------------------------
class BlasInfo {
public: //temp
    VkDevice device = VK_NULL_HANDLE;
    VkAccelerationStructureGeometryKHR ObjToGeometry(MeshObject& obj);
    VkAccelerationStructureGeometryKHR        asGeometry{};
    VkAccelerationStructureBuildGeometryInfoKHR geomInfo{};
public:
    VkAccelerationStructureBuildSizesInfoKHR    sizeInfo{};  //out
    VkAccelerationStructureBuildRangeInfoKHR   rangeInfo{};  //out
    //VkAccelerationStructureBuildRangeInfoKHR* pRangeInfo=&rangeInfo;
    BlasInfo(VkDevice device, MeshObject obj);
    ~BlasInfo(){}
    auto getGeomInfo(VkAccelerationStructureKHR structure, VkBuffer scratch);
};
//--------------------------------------------------------------------------------------------------

//---------------------------------------------BLAS-------------------------------------------------
class BLAS {
    VkPhysicalDevice gpu    = VK_NULL_HANDLE;
    VkDevice         device = VK_NULL_HANDLE;

    void BuildObj(CCmd& cmd, MeshObject obj, bool compact=true);
    void Compact(CCmd& cmd, ABO& blas, VkDeviceSize cmpSize);

public:
    MeshList mesh_list;
    BLASList blas_list;

    BLAS(){}
    ~BLAS(){Destroy();}

    void Init(VkPhysicalDevice gpu, VkDevice device);
    uint32_t AddMesh(VBO& vbo, IBO& ibo, UBO& ubo, bool isOpaque=true);  //return index
    uint32_t AddMesh(MeshObject obj);

    void Build(CCmd& cmd, bool compact=true);  // build blas_list from mesh_list
    void Destroy();

    ABO& operator[](uint32_t i) {return blas_list[i];}
    uint32_t count(){return blas_list.size();}
    operator MeshList& () { return mesh_list; }
};

//--------------------------------------------------------------------------------------------------

#endif
