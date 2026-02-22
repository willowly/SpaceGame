#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D shadowMaps[];

#include "ui_push_constant.hlsl"

void main() {
    outColor = texture(shadowMaps[0],texCoord*2.0f) * push.color;

    gl_FragDepth = 0.0f;

    //outColor = vec4(1,1,1,0.5);
}