#version 450

vec2 positions[4] = vec2[](
    vec2(-1,  1),
    vec2( 1,  1),
    vec2(-1, -1),
    vec2( 1, -1)
);


#include "scene_data.hlsl"

#include "push_constant.hlsl"


void main() {
    uint frameIndex = push.frameIndex;

    gl_Position = sceneData[frameIndex].screen * vec4(positions[gl_VertexIndex], 0.0, 1.0);
}