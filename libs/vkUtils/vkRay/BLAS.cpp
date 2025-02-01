#include "BLAS.h"

//#define SIZE(VECTOR) (uint32_t)VECTOR.size()

//---------------------------------------------QUERY------------------------------------------------
Query::Query(CCmd& cmd, VkAccelerationStructureKHR* as, VkQueryType query_type) {
    device = cmd.device;
    VkQueryPoolCreateInfo query_pool_info = {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    query_pool_info.queryType  = query_type; // VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
    query_pool_info.queryCount = 1;
    vkCreateQueryPool(device, &query_pool_info, nullptr, &query_pool);
    vkResetQueryPool(device, query_pool, 0, 1);
    vkCmdWriteAccelerationStructuresPropertiesKHR(cmd, 1, as, query_type, query_pool, 0);
}

VkDeviceSize Query::GetResult() {
    uint32_t data = 0;
    vkGetQueryPoolResults(device, query_pool, 0, 1, 4, &data, 4, VK_QUERY_RESULT_WAIT_BIT);
    return data;
}

Query::~Query(){vkDestroyQueryPool(device, query_pool, nullptr);}
//--------------------------------------------------------------------------------------------------
//-------------------------------------------BlasInfo-----------------------------------------------
VkAccelerationStructureGeometryKHR BlasInfo::ObjToGeometry(MeshObject& obj) {
    VkAccelerationStructureGeometryKHR geometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
    geometry.flags                            = obj.isOpaque ? VK_GEOMETRY_OPAQUE_BIT_KHR : 0;
    geometry.geometryType                     = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
    geometry.geometry.triangles               = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR};
    geometry.geometry.triangles.vertexFormat  = VK_FORMAT_R32G32B32_SFLOAT; // 3xfloat32 for vertices
    geometry.geometry.triangles.vertexData    = deviceAddress(device, obj.vertexBuffer);
    geometry.geometry.triangles.vertexStride  = obj.vertexStride;
    geometry.geometry.triangles.maxVertex     = obj.vertexCount; //-1??
    geometry.geometry.triangles.indexType     = VK_INDEX_TYPE_UINT32;       // 32-bit indices
    geometry.geometry.triangles.indexData     = deviceAddress(device, obj.indexBuffer);
    geometry.geometry.triangles.transformData = {0}; //null pointer indicates identity transform
    //printf("geometry vrtcnt=%d\n", obj.vertexCount);
    return geometry;
}

BlasInfo::BlasInfo(VkDevice device, MeshObject obj) {
    this->device = device;
    asGeometry = ObjToGeometry(obj);
    uint32_t primCount = obj.indexCount/3;

    geomInfo = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
    sizeInfo = {VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
    rangeInfo= {primCount, 0,0,0};

    VkFlags flags = 0;
  //flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    flags |= VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
    flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
  //flags |= VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR;
    flags |= VK_BUILD_ACCELERATION_STRUCTURE_LOW_MEMORY_BIT_KHR;

    geomInfo.flags                    = flags;
    geomInfo.type                     = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    geomInfo.mode                     = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;  // build vs update
    geomInfo.srcAccelerationStructure = VK_NULL_HANDLE;                                  // for update mode
    geomInfo.dstAccelerationStructure = VK_NULL_HANDLE;                                  // filled later
    geomInfo.geometryCount            = 1;            //(uint32_t)geometries.size();
    geomInfo.pGeometries              = &asGeometry;  //          geometries.data();
    geomInfo.ppGeometries             = 0;
    //geomInfo.scratchData = // Set later, in getGeomInfo

    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
                                            &geomInfo, &primCount, &sizeInfo);
}

auto BlasInfo::getGeomInfo(VkAccelerationStructureKHR structure, VkBuffer scratch) {
    geomInfo.dstAccelerationStructure = structure;
    geomInfo.scratchData = deviceAddress(device, scratch);
    return geomInfo;
}
//--------------------------------------------------------------------------------------------------

//---------------------------------------------BLAS-------------------------------------------------

void BLAS::Init(VkPhysicalDevice gpu, VkDevice device) {
    this->gpu    = gpu;
    this->device = device;
     mesh_list.clear();
    vkGetAccelerationStructureBuildSizesKHR  = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(device, "vkGetAccelerationStructureBuildSizesKHR");
    vkCreateAccelerationStructureKHR         = (PFN_vkCreateAccelerationStructureKHR)       vkGetDeviceProcAddr(device, "vkCreateAccelerationStructureKHR");
    vkGetBufferDeviceAddressKHR              = (PFN_vkGetBufferDeviceAddressKHR)            vkGetDeviceProcAddr(device, "vkGetBufferDeviceAddressKHR");
    vkCmdBuildAccelerationStructuresKHR      = (PFN_vkCmdBuildAccelerationStructuresKHR)    vkGetDeviceProcAddr(device, "vkCmdBuildAccelerationStructuresKHR");
}

uint32_t BLAS::AddMesh(VBO& vbo, IBO& ibo, UBO& ubo, bool is_opaque) {
    ASSERT(vbo.count > 0,   "VKRay: VBO has not been initialized.\n");
    ASSERT(ibo.count > 0,   "VKRay: IBO has not been initialized.\n");
    ASSERT(ubo.count > 0,   "VKRay: UBO has not been initialized.\n");
    ASSERT(ibo.stride == 4, "VKRay: IBO must use 32bits per index.\n");
    MeshObject obj{};
    obj.vertexBuffer  = vbo.buffer;
    obj.vertexCount   = vbo.count;
    obj.vertexStride  = vbo.stride;
    obj.indexBuffer   = ibo.buffer;
    obj.indexCount    = ibo.count;
    obj.uniformBuffer = ubo.buffer;
    obj.isOpaque = is_opaque;
    return AddMesh(obj);
}

uint32_t BLAS::AddMesh(MeshObject obj) {
    mesh_list.push_back(obj);
    return mesh_list.size()-1;  //return index
}

void BLAS::Build(CCmd& cmd, bool compact) {
    blas_list.reserve(mesh_list.size());
    for(const auto& obj : mesh_list) {
        BuildObj(cmd, obj, compact);
    }
    //mesh_list.clear();
}

void BLAS::BuildObj(CCmd& cmd, MeshObject obj, bool compact) {
    BlasInfo bi(device, obj);
    VkDeviceSize scratchSize = bi.sizeInfo.buildScratchSize;
    VkDeviceSize blasSize    = bi.sizeInfo.accelerationStructureSize;

    SBO scratch(scratchSize);
    ABO& blas = blas_list.emplace_back(blasSize, ABO::BLAS);
    auto gi = bi.getGeomInfo(blas.structure, scratch);
    auto* pRangeInfo = &bi.rangeInfo;

    cmd.Begin();
      vkCmdBuildAccelerationStructuresKHR(cmd, 1, &gi, &pRangeInfo);
      //Barrier(cmd);
      mem_barrier(cmd, VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
                       VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR);
      Query query(cmd, &blas.structure, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR);  // query compacted size
    cmd.End(true);

    VkDeviceSize cmpSize = query.GetResult();
    if(compact) Compact(cmd, blas, cmpSize);
}

void BLAS::Compact(CCmd& cmd, ABO& blas, VkDeviceSize cmpSize) {
    printf("Blas compact: %lu/%lu (%lu%%)\n", (long)cmpSize, (long)blas.size(), (long)(cmpSize*100/blas.size()));

    ABO old = std::move(blas);          // save old blas data
    blas.Allocate(cmpSize, ABO::BLAS);  // reallocate blas to compact size

    VkCopyAccelerationStructureInfoKHR cpyInfo{VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR};
    cpyInfo.src = old.structure;
    cpyInfo.dst = blas.structure;
    cpyInfo.mode= VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;

    cmd.Begin();
        vkCmdCopyAccelerationStructureKHR(cmd, &cpyInfo);
    cmd.End(true);
}

void BLAS::Destroy() {
    blas_list.clear();
    mesh_list.clear();
}
//--------------------------------------------------------------------------------------------------
