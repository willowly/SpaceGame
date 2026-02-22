#pragma once
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_buffer_reference : require

#ifndef MATERIAL_DATA
    layout(buffer_reference) buffer MaterialData;
#endif

layout( push_constant ) uniform constants
{
	mat4 modelMatrix;
    uint frameIndex;
    MaterialData material;
    vec4 color;
    // extra gap here?
    
} push;