#ifndef TLAS_H
#define TLAS_H

#include "BLAS.h"
#include "matrix.h"

const VkGeometryInstanceFlagsKHR GI_FLAGS = 0
    | VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR
   // | VK_GEOMETRY_INSTANCE_TRIANGLE_FRONT_COUNTERCLOCKWISE_BIT_KHR
   // | VK_GEOMETRY_INSTANCE_FORCE_OPAQUE_BIT_KHR
   // | VK_GEOMETRY_INSTANCE_FORCE_NO_OPAQUE_BIT_KHR
;

const VkBuildAccelerationStructureFlagsKHR AS_FLAGS = 0
      | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR
      //| VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR
      | VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR
      //| VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR
      //| VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR
;

//----------------------------- TLAS -------------------------------
typedef std::vector<VkAccelerationStructureInstanceKHR> ASInstances;

class TLAS {
    VkPhysicalDevice gpu    = VK_NULL_HANDLE;
    VkDevice         device = VK_NULL_HANDLE;
    //VkCommandBuffer  cmd    = VK_NULL_HANDLE;
    VkBuildAccelerationStructureFlagsKHR flags = AS_FLAGS;
//public:
    CvkBuffer instances;
    ABO topAS;

    ASInstances inst_list;
    uint32_t AddInst(VkAccelerationStructureKHR blas, uint32_t hitGroupIndex, VkGeometryInstanceFlagsKHR flags=GI_FLAGS);

public:
    TLAS(){};
    ~TLAS(){Destroy();};
    void Init(VkPhysicalDevice gpu, VkDevice device);
    void AddInstances(BLAS& blas);
    void UpdateInst(uint32_t id, mat4& m, bool visible=true);
    void Build(CCmd& cmd, bool onHost=false);
    void Destroy();

    VkAccelerationStructureInstanceKHR& operator[](uint32_t i) {return inst_list[i];}
    uint32_t count(){return inst_list.size();}
    operator VkAccelerationStructureKHR& () { return topAS.structure; }
};
//------------------------------------------------------------------

#endif
