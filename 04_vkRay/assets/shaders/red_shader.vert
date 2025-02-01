#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
#include "bindings.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTC;

layout(location = 0) out vec2 outTC;

void main() {
    gl_Position = camera.proj * camera.view * model.matrix * vec4(inPosition, 1.0);
    outTC = inTC;
}
