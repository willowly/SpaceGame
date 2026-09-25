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

void main() {
    MaterialData material = push.material;

    vec4 texture = texture(texSampler[material.textureID],texCoord);
    if(texture.a < 0.1) {
        discard;
    }
    vec3 albedo = texture.rgb * toLinear(material.color.rgb);
    
    outColor = vec4(albedo, 1.0);


}