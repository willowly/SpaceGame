#version 450

#include "scene_data.hlsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

#include "color_helper.hlsl"

void main() {
    MaterialData material = push.material;

    vec3 lightDir = normalize(vec3(0.5,1.0,0.1));
    vec3 lightColor = vec3(3,3,3);
    vec3 ambient = vec3(0.3,0.3,0.5);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = diff * lightColor;

    vec3 absNormal = abs(normal);

    float texScale = 0.3f;

    vec3 albedoX = texture(texSampler[material.textureID],position.yz*texScale).rgb * absNormal.x;
    vec3 albedoY = texture(texSampler[material.textureID],position.xz*texScale).rgb * absNormal.y;
    vec3 albedoZ = texture(texSampler[material.textureID],position.xy*texScale).rgb * absNormal.z;

    vec3 objectColor = ((albedoX+albedoY+albedoZ)/(absNormal.x+absNormal.y+absNormal.z)) * toLinear(material.color.rgb);
    vec3 result = (ambient + diffuse) * objectColor;
    outColor = vec4(result, 1.0);
    
    //outColor = vec4(texCoord,0,0);

}