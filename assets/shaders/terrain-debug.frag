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

    

    
    outColor = vec4(1.0,1.0,1.0,1.0);
    
    //outColor = vec4(texCoord,0,0);

}