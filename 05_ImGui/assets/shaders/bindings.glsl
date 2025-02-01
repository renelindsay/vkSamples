
// MATERIAL
struct Material { 
    vec4 albedo;
    vec4 emission;
    vec4 normal;
    vec4 orm;
    
    //int  inx_albedo;
    //int  inx_emission;
    //int  inx_normal;
    //int  inx_orm;
};

// MODEL UBO
layout(binding = 0) uniform Model {
    mat4     matrix;
    Material material;
} model;


// CAMERA UBO
layout(binding = 1) uniform Camera {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
    uint flags;
} camera;

// TEXTURES
layout(binding = 2) uniform sampler2D tex_albedo;
layout(binding = 3) uniform sampler2D tex_emission;
layout(binding = 4) uniform sampler2D tex_normal;
layout(binding = 5) uniform sampler2D tex_orm;

//layout(binding = 6) uniform samplerCube tex_cubemap;