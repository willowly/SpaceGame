#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

#define MATERIAL_DATA

layout(scalar, buffer_reference, buffer_reference_align = 4) readonly buffer MaterialData
{
    uint textureID; 
    vec3 color;
};