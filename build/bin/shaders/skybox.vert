#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#include "scene_data.hlsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in int inFace;

layout(location = 0) out int outFace;
layout(location = 1) out vec2 outTexCoord;

#include "push_constant.hlsl"

void main() {

    uint frameIndex = push.frameIndex;
    mat4 modelMatrix = push.modelMatrix;

    gl_Position = sceneData[frameIndex].proj * sceneData[frameIndex].view * modelMatrix * vec4(inPosition, 1.0);
    gl_Position.z = gl_Position.w;
    outFace = inFace;
    outTexCoord = inTexCoord;
    
}