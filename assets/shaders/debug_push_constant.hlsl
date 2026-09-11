#pragma once
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
    vec4 color;
    // extra gap here?
    
} push;