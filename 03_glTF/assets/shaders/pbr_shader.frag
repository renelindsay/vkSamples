#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
#include "bindings.glsl"
//precision mediump float;

//layout(binding = 2) uniform sampler2D   tex_albedo;
//layout(binding = 3) uniform sampler2D   tex_normal;
//layout(binding = 4) uniform sampler2D   tex_emission;
//layout(binding = 5) uniform sampler2D   tex_orm;
layout(binding = 6) uniform samplerCube tex_cubemap;

layout(location = 0) in vec2 tc;           // Texture coordinates
layout(location = 1) in vec3 in_norm_vec;  // surface normal vector
layout(location = 2) in vec3 in_eye_vec;   // camera-to-surface vector
layout(location = 3) in vec3 up;           // model's up vector

layout(location = 0) out vec4 outColor;

void main() {
    vec4 albedo   = texture(tex_albedo,   tc) * model.material.albedo;
    vec4 normal   = texture(tex_normal,   tc);
    vec4 emission = texture(tex_emission, tc) * model.material.emission;
    vec4 ORM      = texture(tex_orm,      tc) * model.material.orm;

    float AO        = ORM.r * model.material.orm.r;
    float roughness = ORM.g * model.material.orm.g;
    float metalness = ORM.b * model.material.orm.b;
    uint flags = camera.flags;

    vec3 norm_vec = normalize(in_norm_vec);
    vec3 eye_vec  = normalize(in_eye_vec);

    // NORMAL MAP
    //if (model.material.inx_normal < 0) normal = vec4(0,0,255,0);  // if no normal texture, default to up
    
    vec3 norm = norm_vec;  // per-vertex normal (no normal-map)
    if(normal!=vec4(1)) {  // per-fragment normal, using normal-map
        //vec3 up = normalize(-ubo_model[1].xyz);
        vec3 zAxis = norm_vec;
        vec3 xAxis = normalize(cross(up, zAxis));
        vec3 yAxis = normalize(cross(zAxis, xAxis));
        mat3 tangent_space = mat3(xAxis, yAxis, zAxis);
        vec3 normalvec = normalize(normal * 2.0 - 1.0).xyz; // rgb->xyz
        norm = normalize(tangent_space * normalvec);
    }
    vec3 ref_vec = reflect(eye_vec, norm);  // reflection vector (per fragment)

    // DIFFUSE LIGHT    
    // Blur 4 mipmap levels together, to get a diffuse light approximation
    int levels = textureQueryLevels(tex_cubemap);
    vec4 diffuse  = textureLod(tex_cubemap, norm, levels-1);
         diffuse += textureLod(tex_cubemap, norm, levels-2);
         diffuse += textureLod(tex_cubemap, norm, levels-3);
         diffuse += textureLod(tex_cubemap, norm, levels-4);
    diffuse /=4.0;

    // SPECULAR LIGHT
    //float LOD = textureQueryLod(tex_cubemap, ref_vec).x;
    //vec4 specular  = texture(tex_cubemap, ref_vec, roughness * (levels-LOD));
    vec4 specular  = texture(tex_cubemap, ref_vec, roughness * (levels));

    // FRESNEL
    float cosTheta = dot(ref_vec, eye_vec);
    float c = clamp(cosTheta, 0.0, 1.0);
    float fresnel = pow(c, 5.0);
    float kS = 0.04;       // specular fraction
    float kD = 1.0 - kS;   // diffuse fraction
    kS += (kD * fresnel);  // more reflective at sharp angles

    // COMPOSE FINAL COLOR
    vec4 white     = vec4(1,1,1,1);
    vec4 metal     = mix(albedo, white, fresnel) * specular;
    vec4 non_metal = mix(albedo * diffuse, specular, kS);
    outColor       = mix(non_metal, metal, metalness) * AO + emission;
    
    // FLAGS
    if(flags == 1) outColor = diffuse;                         // diffuse light from surroundings
    if(flags == 2) outColor = specular;                        // specular reflections
    if(flags == 3) outColor = albedo;                          // surface color(non_metal) or tint(metal)
    if(flags == 4) outColor = vec4(norm_vec,1);                // per-vertex normal   (no normal map)
    if(flags == 5) outColor = vec4(norm,1);                    // normal map texture  (tangent-space)
    if(flags == 6) outColor = vec4(AO);                        // Ambient occlusion
    if(flags == 7) outColor = vec4(0,roughness,metalness,1);   // roughness(green) metalness(blue)
    if(flags == 8) outColor = vec4(fresnel);                   // increased surface reflectivity at high incidence angles
    
    //outColor = specular;
}
