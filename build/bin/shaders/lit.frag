#version 410

in vec3 color;
in vec2 texCoord;
in vec3 normal;

out vec4 FragColor;

uniform sampler2D mainTex;

void main()
{
    vec3 lightDir = normalize(vec3(0.5,1.0,0.1));
    vec3 lightColor = vec3(1,1,1);
    vec3 ambient = vec3(0.1,0.1,0.2);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = diff * lightColor;
    vec3 objectColor = texture(mainTex,texCoord).rgb;
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}