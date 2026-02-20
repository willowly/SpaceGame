#version 450
#extension GL_EXT_nonuniform_qualifier : enable

#include "scene_data.hlsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in int inTextureID;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out int outTextureID;
layout(location = 3) out vec4 lightSpacePosition;

#include "push_constant.hlsl"

#include "shadow_maps.hlsl"

void main() {

    uint frameIndex = push.frameIndex;
    mat4 modelMatrix = push.modelMatrix;

    gl_Position = sceneData[frameIndex].proj * sceneData[frameIndex].view * modelMatrix * vec4(inPosition, 1.0);
    outNormal = normalize(mat3(transpose(inverse(modelMatrix))) * inNormal);
    outTexCoord = inTexCoord;
    outTextureID = inTextureID;
    lightSpacePosition = getLightSpacePos(frameIndex,modelMatrix,inPosition);
    
}