#version 450

#include "scene_data.hlsl"

#include "push_constant.hlsl"

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;

void main() {
    
    uint frameIndex = push.frameIndex;
    outColor = vec4(1.0f,0.5,0.0f,1.0);
    
    //outColor = vec4(texCoord,0,0);

}