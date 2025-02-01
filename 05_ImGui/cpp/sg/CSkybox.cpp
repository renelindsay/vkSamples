#include "CSkybox.h"


//   1------0
//   |      | \
//   |      |   \
//   3------2     \
//    \     5-\----4
//      \   |   \  |
//        \ |     \|
//          7------6

void CBox::Init() {
    LOGI("INIT: %s\n", name.c_str());
    assert(pipeline && "Pipeline not set.");

    static const vec3 cubevert[] = {{1,1,1},{-1,1,1}, {1,-1,1}, {-1,-1,1},  {1,1,-1}, {-1,1,-1}, {1,-1,-1}, {-1,-1,-1}};
    static const vec3 cubenorm[] = {{0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}};
    static const vec2 face_tc[]  = {{1,1}, {0,1}, {1,0}, {0,0}};
    static const uint cubeinx[]  = {0,1,2,3, 4,5,6,7,  0,2,4,6, 3,1,7,5,  1,0,5,4, 2,3,6,7};

    std::vector<uint>   indices(36);
    std::vector<Vertex> vertices(24);
    repeat(24) {
        uint inx = cubeinx[i];
        vertices[i] = {cubevert[inx], cubenorm[i/4], face_tc[i&3]};
    }

    auto AddQuad = [&](uint v0, uint v1, uint v2, uint v3) {
        if(flipped) indices.insert(indices.end(), {v1,v0,v2,v2,v3,v1});
        else        indices.insert(indices.end(), {v0,v1,v2,v3,v2,v1});
    };

    AddQuad( 0, 1, 2, 3);    // top
    AddQuad( 4, 6, 5, 7);    // btm
    AddQuad( 8, 9,10,11);    // rgt
    AddQuad(12,13,14,15);    // lft
    AddQuad(16,17,18,19);    // bck
    AddQuad(20,21,22,23);    // fnt

    vbo.Data(vertices.data(), 24);
    ibo.Data(indices.data(), (uint32_t)indices.size());
}

void CBox::Bind() {
    assert(pipeline && "Pipeline not set.");
    CShader& shader = pipeline->shader;

    if(!ubo.size()) ubo.Allocate(sizeof(ubo_data));
    shader.Bind("model", ubo);

    if(cubemap) shader.Bind("tex_cubemap", cubemap);
    shader.UpdateDescriptorSets(descriptorSets);
}

void CBox::Draw() {
    //Update UBO
    ubo_data.matrix = worldMatrix;
    ubo.Update(&ubo_data);

    //VkPipelineLayout pipelineLayout = pipeline->shader.GetPipelineLayout();
    //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);
    //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, descriptorSets, 0, nullptr);
    pipeline->Bind(commandBuffer, descriptorSets);

    // Geometry
    vkCmdBindVertexBuffer  (commandBuffer, vbo);
    vkCmdBindIndexBuffer   (commandBuffer, ibo, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed       (commandBuffer, ibo.Count(), 1, 0, 0, 0);
}

//--------------------------------------------------------------------

void CSkybox::Draw() {
    float farplane = cam_uniform.proj.m23 / (cam_uniform.proj.m22 + 1.0);
    vec4 cam_pos = cam_uniform.viewInverse.row.position4;
    worldMatrix.Clear();
    worldMatrix.Scale(farplane/sqrtf(3));
    worldMatrix.row.position4 = cam_pos; // center on camera location
    CBox::Draw();
}

//----------------------------------------------------------------------
