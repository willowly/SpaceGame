#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 outTexCoord;


#include "scene_data.hlsl"

#include "ui_push_constant.hlsl"


void main() {
    uint frameIndex = push.frameIndex;
    mat4 modelMatrix = push.modelMatrixBuffer.matrices[gl_InstanceIndex];

    gl_Position = sceneData[frameIndex].screen * modelMatrix * vec4(position, 0.0, 1.0);
    gl_Position.z = 0.0;

    outTexCoord = uv;
}