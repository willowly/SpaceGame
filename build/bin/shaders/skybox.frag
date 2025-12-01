#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#include "scene_data.hlsl"


layout(location = 0) flat in int face;
layout(location = 1) out vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#define MATERIAL_DATA

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer MaterialData
{
    uint top; 
};

#include "push_constant.hlsl"

#include "color_helper.hlsl"

void main() {
    MaterialData material = push.material;


    outColor = texture(texSampler[material.top],texCoord);
    
    //outColor = vec4(texCoord,0,0);

}