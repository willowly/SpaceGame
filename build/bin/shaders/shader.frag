#version 450

#include "scene_data.hlsl"

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

void main() {

    
    MaterialData material = push.material;
    //outColor = colors[sceneData[push.frameIndex].testIndex];
    //outColor = push.frameIndex == 0 ? vec4(0,0,1,1) : vec4(0,1,0,1);
    outColor = texture(texSampler[material.textureID], fragTexCoord);
}