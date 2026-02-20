#version 450


#include "scene_data.hlsl"

layout(location = 0) in vec3 normal;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "lit_material_data.hlsl"

#include "push_constant.hlsl"

#include "color_helper.hlsl"

void main() {
    MaterialData material = push.material;
    uint frameIndex = push.frameIndex;

    vec3 lightDir = sceneData[frameIndex].mainLightDirection;
    vec3 lightColor = sceneData[frameIndex].mainLightColor;
    vec3 ambient = sceneData[frameIndex].ambientLightColor;


    vec3 diffuse = lightColor;

    vec4 textureColor = texture(texSampler[material.textureID],texCoord);
    if(textureColor.a < 0.5f) {
        discard;
    }
    vec4 objectColor = textureColor * toLinear(material.color);
    outColor = objectColor;
    
    
    //outColor = vec4(texCoord,0,0);

}