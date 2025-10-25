#extension GL_EXT_buffer_reference : require

#define MATERIAL_DATA

layout(std430, buffer_reference, buffer_reference_align = 8) readonly buffer MaterialData
{
    uint textureID;
    vec3 color;
};