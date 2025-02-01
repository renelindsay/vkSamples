#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
//#include "bindings.glsl"

layout(binding = 0) uniform Model {
    mat4     matrix;
    //Material material;

} model;

layout(binding = 1) uniform Camera {
    mat4 view;
    mat4 proj;
    mat4 viewInverse;
    mat4 projInverse;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTC;

layout(location = 0) out vec3 direction;

void main() {
    mat4 m = model.matrix;
    mat4 v = camera.view;
    mat4 p = camera.proj;

    vec4 dir4 = vec4(inPosition, 1.0);
    direction = (dir4 * m).xyz;
    gl_Position = p * v * m * dir4;
}
