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

    vec3 lightDir = normalize(vec3(0.5,1.0,0.1));
    vec3 lightColor = vec3(3,3,3);
    vec3 ambient = vec3(0.3,0.3,0.5);


    vec3 diffuse = lightColor;

    vec4 textureColor = texture(texSampler[material.textureID],texCoord);
    if(textureColor.a < 0.5f) {
        discard;
    }
    vec4 objectColor = textureColor * toLinear(material.color);
    outColor = objectColor;
    
    
    //outColor = vec4(texCoord,0,0);

}