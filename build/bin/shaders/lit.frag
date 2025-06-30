#version 410

in vec2 texCoord;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D mainTex;
uniform vec3 color;

void main()
{
    vec3 lightDir = normalize(vec3(0.5,1.0,0.1));
    vec3 lightColor = vec3(1,1,1);
    vec3 ambient = vec3(0.3,0.3,0.5);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = diff * lightColor;
    vec3 objectColor = texture(mainTex,texCoord).rgb * color;
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}