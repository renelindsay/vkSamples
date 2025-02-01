#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

struct Camera {
    mat4  view;
    mat4  proj;
    mat4  viewInverse;
    mat4  projInverse;
    uint  flags;
    int   sky;
    uint  random;
};

struct RayPayload {
    //int hit;

    float distance;
    //vec3 hitpos;
    //vec3 normal;
    //vec3 diffuse;
    //vec4 albedo;
    //vec3 illum;
    //vec3 orm;
    
    vec3 color;
};

layout(location = 0) rayPayloadInEXT RayPayload payload;

layout(binding = 2, set = 0) uniform Cameras  { Camera camera; }; // camera ubo
layout(binding = 6, set = 0) uniform samplerCube[] cubemaps;

void main() {
    vec3 color = vec3(0,0,0);
    vec3 dir = gl_WorldRayDirectionEXT;
    //payload.color = texture(cubemaps[camera.sky], dir).xyz;
   
    int levels = textureQueryLevels(cubemaps[camera.sky]);
    color += textureLod(cubemaps[camera.sky], dir, levels-4).xyz;
    //color += textureLod(cubemaps[camera.sky], dir, levels-3).xyz;   
    payload.color = color/1.0;
}
