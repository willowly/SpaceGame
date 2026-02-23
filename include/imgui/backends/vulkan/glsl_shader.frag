#version 450 core
#include "color_helper.hlsl"

layout(location = 0) out vec4 fColor;

layout(set=0, binding=0) uniform sampler2D sTexture;

layout(location = 0) in struct {
    vec4 Color;
    vec2 UV;
} In;

void main()
{
    fColor = toLinear(In.Color) * texture(sTexture, In.UV.st);
}
