#include "Material.h"

void Material::Bind(CShader& shader) {  // see CMesh::Bind
    shader.Bind("tex_albedo",   texture.albedo);
    shader.Bind("tex_emission", texture.emission);
    shader.Bind("tex_normal",   texture.normal);
    shader.Bind("tex_orm",      texture.orm);
}
