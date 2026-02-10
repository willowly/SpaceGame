#version 450

#include "scene_data.hlsl"

#include "push_constant.hlsl"

#extension GL_EXT_debug_printf : enable


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;

vec2 positions[6] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5),
    vec2(0.0, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5)
);

void main() {

    // uint frameIndex = push.frameIndex;
    // mat4 modelMatrix = push.modelMatrix;

    gl_Position = vec4(positions[0], 0.0, 1.0);

    // debugPrintfEXT("vertex kernel v:%i f:%i s:%p",gl_VertexIndex,0,sceneData);
}