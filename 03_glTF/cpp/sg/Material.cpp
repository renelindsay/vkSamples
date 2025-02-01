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
