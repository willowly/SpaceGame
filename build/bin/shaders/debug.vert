#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#include "scene_data.hlsl"

layout(location = 0) in vec3 inPosition;

#include "debug_push_constant.hlsl"

void main() {

    uint frameIndex = push.frameIndex;
    mat4 modelMatrix = push.modelMatrix;

    gl_Position = sceneData[frameIndex].proj * sceneData[frameIndex].view * modelMatrix * vec4(inPosition, 1.0);
    
}