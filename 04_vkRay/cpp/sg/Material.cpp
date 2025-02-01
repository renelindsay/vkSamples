#include "Material.h"
#include "Buffers.h"

void Material::Bind(CShader& shader) {
    shader.Bind("tex_albedo",   texture.albedo);
    shader.Bind("tex_emission", texture.emission);
    shader.Bind("tex_normal",   texture.normal);
    shader.Bind("tex_orm",      texture.orm);
}
/*
void Material::Update(uboData& ubo_data) {
    //ubo_data.matrix  = worldMatrix;
    ubo_data.color[0] = color.albedo;
    ubo_data.color[1] = color.emission;
    ubo_data.color[2] = color.normal;
    ubo_data.color[3] = color.orm;
    //ubo.Update(&ubo_data);
}
*/


//---------------------------------------------
/*
uint32_t ImageList::AddImage(CvkImage* img) {  //return image index
    if(!img) return -1;
    vkImages.push_back(img);
    return (uint32_t)vkImages.size()-1;
}

void ImageList::UpdateImage(uint32_t inx, CvkImage& img) {
    //vkDeviceWaitIdle(device);
    ASSERT(inx < vkImages.size(), "SetImage: rtMaterials does not contain index: %d", inx);
    *vkImages[inx] = std::move(img);
    //vkray.BindImages();
    //vkray.UpdateSetContents();
}

void ImageList::AddMaterial(Material& mat, uboData& ubo_data) {
    //If there's an emission texture, turn on emission
    if(mat.texture.emission) mat.color.emission = {1,1,1,1};

    ubo_data.color[0] = mat.color.albedo;
    ubo_data.color[1] = mat.color.emission;
    ubo_data.color[2] = mat.color.normal;
    ubo_data.color[3] = mat.color.orm;
    ubo_data.texid[0] = AddImage(mat.texture.albedo);
    ubo_data.texid[1] = AddImage(mat.texture.emission);
    ubo_data.texid[2] = AddImage(mat.texture.normal);
    ubo_data.texid[3] = AddImage(mat.texture.orm);
}
*/
