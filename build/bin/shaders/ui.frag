#version 450

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler[];

#include "ui_push_constant.hlsl"

void main() {
    vec2 spriteCoord = texCoord * push.spriteSize;
    spriteCoord += push.spriteOffset;
    outColor = texture(texSampler[push.textureID],spriteCoord) * push.color;

    gl_FragDepth = 0.0f;

    //outColor = vec4(1,1,1,0.5);
}