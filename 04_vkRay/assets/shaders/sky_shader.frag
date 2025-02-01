#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(binding = 2) uniform sampler2D tex_Albedo;
//layout(binding = 3) uniform sampler2D tex_Normal;
//layout(binding = 4) uniform sampler2D tex_Emission;
//layout(binding = 5) uniform sampler2D tex_ORM;

layout(binding = 6) uniform samplerCube tex_cubemap;

layout(location = 0) in vec3 direction;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 cubemap = texture(tex_cubemap, direction);
    outColor = cubemap;
}
