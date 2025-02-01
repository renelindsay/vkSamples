#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "utils.glsl"

struct Camera {
    mat4  view;
    mat4  proj;
    mat4  viewInverse;
    mat4  projInverse;
    uint  flags;
    int   sky;
    uint  random;
};

struct Light {
    bool enabled;
    vec4 color;
    vec4 position;  // or direction, if w=0
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
layout(binding = 7, set = 0) uniform Lights   { Light light;   };

void main() {    
    vec3 dir = gl_WorldRayDirectionEXT;
    payload.color = texture(cubemaps[camera.sky], dir).xyz;
   
    //int levels = textureQueryLevels(cubemaps[camera.sky]);
    //payload.color = textureLod(cubemaps[camera.sky], dir, levels-4).xyz;
    
    // Mark sun in red
    //vec3 ldir = light.position.xyz;
    //float ldot = dot(dir, ldir);
    //if(ldot > cos(1.0*toRad)) payload.color = vec3(1,0,0);
}