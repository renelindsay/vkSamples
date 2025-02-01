#version 460
#extension GL_ARB_separate_shader_objects : enable
#extension GL_GOOGLE_include_directive    : enable
#include "bindings.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTC;

layout(location = 0) out vec2 tc;
layout(location = 1) out vec3 norm_vec;
layout(location = 2) out vec3 eye_vec;
layout(location = 3) out vec3 up;

void main() {
    norm_vec = normalize(mat3(model.matrix) * inNormal);
    vec4 vrtPos = model.matrix * vec4(inPosition, 1.0);

    //mat4 invView = inverse(camera.view);
    mat4 invView = camera.viewInverse;
    vec3 camPos = invView[3].xyz / invView[3].w;
    eye_vec = normalize(vrtPos.xyz - camPos);
    
    up = normalize(-model.matrix[1].xyz);           //  tangents

    gl_Position = camera.proj * camera.view * model.matrix * vec4(inPosition, 1.0);
    tc = inTC;
}
