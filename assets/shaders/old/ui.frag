#version 410

in vec2 texCoord;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D mainTex;
uniform vec4 color;

void main()
{
    vec4 result = texture(mainTex,texCoord) * color;
    FragColor = result;
}