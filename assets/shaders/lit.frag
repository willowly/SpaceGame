#version 450


#include "scene_data.hlsl"

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec4 lightSpacePosition;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

#include "color_helper.hlsl"

#include "shading.hlsl"

void main() {
    MaterialData material = push.material;

    vec3 albedo = texture(texSampler[material.textureID],texCoord).rgb * toLinear(material.color.rgb);
    
    vec3 result = simpleLitShadow(albedo,normal,lightSpacePosition);
    outColor = vec4(result, 1.0);


}