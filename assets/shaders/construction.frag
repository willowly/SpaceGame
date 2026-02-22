#version 450

#include "scene_data.hlsl"

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in flat int textureID;
layout(location = 3) in vec4 lightSpacePosition;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

#include "color_helper.hlsl"

#include "shading.hlsl"

void main() {
    MaterialData material = push.material;
    uint frameIndex = push.frameIndex;

    vec3 objectColor = texture(texSampler[textureID],texCoord).rgb * toLinear(material.color.rgb);

    vec3 result = simpleLitShadow(objectColor,normal,lightSpacePosition);
    outColor = vec4(result, 1.0);
    
    //outColor = vec4(texCoord,0,0);

}