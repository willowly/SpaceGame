#version 450

#include "scene_data.hlsl"

layout(location = 0) out vec4 outColor;

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#define MATERIAL_DATA

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer MaterialData
{
    vec3 color;
};

#include "push_constant.hlsl"

#include "color_helper.hlsl"

void main() {
    MaterialData material = push.material;

    outColor = vec4(material.color, 1.0);
    
    //outColor = vec4(texCoord,0,0);

}