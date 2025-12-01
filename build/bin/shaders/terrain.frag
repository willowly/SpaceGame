#version 450

#include "scene_data.hlsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) flat in ivec4 oreTextureID;
layout(location = 3) in vec4 oreBlend;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

#include "color_helper.hlsl"


vec3 getAlbedo(int textureID,vec3 absNormal) {
    float texScale = 0.3f;
    vec3 albedoX = texture(texSampler[textureID],position.yz*texScale).rgb * absNormal.x;
    vec3 albedoY = texture(texSampler[textureID],position.xz*texScale).rgb * absNormal.y;
    vec3 albedoZ = texture(texSampler[textureID],position.xy*texScale).rgb * absNormal.z;
    return ((albedoX+albedoY+albedoZ)/(absNormal.x+absNormal.y+absNormal.z));
}

void main() {
    MaterialData material = push.material;

    vec3 lightDir = normalize(vec3(0.5,1.0,0.1));
    vec3 lightColor = vec3(3,3,3);
    vec3 ambient = vec3(0.1,0.1,0.13);

    float diff = max((dot(normal, lightDir)/2)+0.5, 0.0);

    // float fresnel = 1.0f-abs(dot(normal, sceneData[push.frameIndex].viewDir));
    // fresnel = pow(fresnel,3);

    vec3 diffuse = diff * lightColor;

    vec3 absNormal = abs(normal);

    

    vec3 albedoA = getAlbedo(oreTextureID.x,absNormal) * oreBlend.x;
    vec3 albedoB = getAlbedo(oreTextureID.y,absNormal) * oreBlend.y;
    vec3 albedoC = getAlbedo(oreTextureID.z,absNormal) * oreBlend.z;

    vec3 albedo = (albedoA+albedoB+albedoC)/(oreBlend.x+oreBlend.y+oreBlend.z);

    vec3 objectColor = albedo * toLinear(material.color.rgb);
    vec3 result = (ambient + diffuse) * objectColor;
    outColor = vec4(result, 1.0);
    
    //outColor = vec4(texCoord,0,0);

}