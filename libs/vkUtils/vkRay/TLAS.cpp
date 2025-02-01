#include "TLAS.h"

VkTransformMatrixKHR vkMatrix(mat4 m) {
    return  {m.m00, m.m01, m.m02, m.m03,
             m.m10, m.m11, m.m12, m.m13,
             m.m20, m.m21, m.m22, m.m23};
}

VkDeviceAddress ASdeviceAddress(VkDevice device, VkAccelerationStructureKHR AS) {
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR, 0, AS};
    return vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
}

void TLAS::Init(VkPhysicalDevice gpu, VkDevice device) {
    this->gpu    = gpu;
    this->device = device;
    inst_list.clear();
}

void TLAS::AddInstances(BLAS& blas) {
    uint cnt = blas.count();
    inst_list.clear();
    inst_list.reserve(cnt);
    repeat(cnt) AddInst(blas[i].structure, 0);
}

uint32_t TLAS::AddInst(VkAccelerationStructureKHR blas, uint32_t hitGroupIndex, VkGeometryInstanceFlagsKHR flags) {
    uint id = inst_list.size();
    VkAccelerationStructureInstanceKHR& inst = inst_list.emplace_back();
    inst.transform                              = vkMatrix(Identity4x4);
    inst.instanceCustomIndex                    = id;                            // gl_InstanceCustomIndexEXT
    inst.mask                                   = 0xFF;
    inst.instanceShaderBindingTableRecordOffset = hitGroupIndex;                 // hit group index
    inst.flags                                  = flags;
    inst.accelerationStructureReference         = ASdeviceAddress(device, blas);
    return id;
}

void TLAS::UpdateInst(uint32_t id, mat4& m, bool visible) {
    auto& inst = inst_list[id];
    inst.transform = vkMatrix(m);
    inst.mask = visible ? 0xff : 0;
}

void TLAS::Build(CCmd& cmd, bool onHost) {
    bool update = !!topAS.structure;
    uint32_t count = inst_list.size();
    //VkDeviceSize SizeInBytes = count * sizeof(VkAccelerationStructureInstanceKHR);  // get total size of inst_list array
    instances.Clear();

    VkDeviceSize ASIsize = sizeof(VkAccelerationStructureInstanceKHR);
    VkFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
    instances.Data(inst_list.data(), count, ASIsize, usage);

    //mem_barrier(*default_allocator, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR);
    //--------------------------------------------------------------------------------------------------

    // Wrap the instances device pointer into a VkAccelerationStructureGeometryKHR.
    VkAccelerationStructureGeometryKHR topASGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    topASGeometry.geometryType            = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    topASGeometry.flags                   = VK_GEOMETRY_OPAQUE_BIT_KHR;
    topASGeometry.geometry.instances      = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR};
    topASGeometry.geometry.instances.data = deviceAddress(device, instances);
    topASGeometry.geometry.instances.arrayOfPointers = VK_FALSE;

    // Find sizes
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    buildInfo.flags         = flags;  //fast trace
    buildInfo.geometryCount = 1;
    buildInfo.pGeometries   = &topASGeometry;
    buildInfo.mode = update ? VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR : VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    if(onHost)vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR, &buildInfo, &count, &sizeInfo);
    else      vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &count, &sizeInfo);

    // Allocate and Create TLAS (unless updating)
    if(update == false) topAS.Allocate(sizeInfo.accelerationStructureSize, ABO::TLAS);

    // Allocate the scratch and TLAS buffers
    SBO scratch;
    if(update) scratch.Allocate(sizeInfo.updateScratchSize);
    else       scratch.Allocate(sizeInfo.buildScratchSize);

    // Update build information
    buildInfo.srcAccelerationStructure  = update ? topAS.structure : VK_NULL_HANDLE;
    buildInfo.dstAccelerationStructure  = topAS.structure;
    buildInfo.scratchData.deviceAddress = scratch.DeviceAddress();

    // Build the TLAS
    VkAccelerationStructureBuildRangeInfoKHR rangeInfo{count, 0,0,0};
    auto* pRangeInfo = &rangeInfo;                                         // Convert offset to pointer-to-offset
    if(onHost) {
        vkBuildAccelerationStructuresKHR(device, 0, 1, &buildInfo, &pRangeInfo);  // Build TLAS on Host (Untested: Not supported on Nvidia 1080)
    }else{
        cmd.Begin();
        vkCmdBuildAccelerationStructuresKHR(cmd, 1, &buildInfo, &pRangeInfo);     // Build TLAS on Device
        cmd.End(true);
    }
    if(update == false) printf("Tlas size: %d\n", (int)topAS.size());
}

void TLAS::Destroy() {
    topAS.Clear();
    instances.Clear();
    inst_list.clear();
}
