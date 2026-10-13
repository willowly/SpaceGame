#ifndef PUSH_CONST
#define PUSH_CONST
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

#ifndef MATERIAL_DATA
    layout(buffer_reference) buffer MaterialData;
#endif

layout(buffer_reference) buffer ModelMatrixBuffer {
    mat4 matrices[];
};

layout( push_constant ) uniform constants
{
	ModelMatrixBuffer modelMatrixBuffer;
    uint frameIndex;
    MaterialData material;
    ivec4 ditherFadeDistances; // x = close fade start y = close fade end, z = far fade start, w = far fade end
} push;

#endif