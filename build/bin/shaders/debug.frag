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

#include "debug_push_constant.hlsl"

#include "color_helper.hlsl"

void main() {
    

    outColor = push.color;

    gl_FragDepth = 0.0f;

}