
#include "shadow_maps.hlsl"

#include "scene_data.hlsl"

#include "push_constant.hlsl"

vec3 simpleLitShadow(vec3 albedo,vec3 normal,vec4 lightSpacePosition) {

    uint frameIndex = push.frameIndex;

    vec3 lightDir = sceneData[frameIndex].mainLightDirection;
    vec3 lightColor = sceneData[frameIndex].mainLightColor;
    vec3 ambient = sceneData[frameIndex].ambientLightColor;

    float diff = max(dot(normal, -lightDir), 0.0);

    vec3 diffuse = diff * lightColor;

    float shadow = getShadow(lightSpacePosition);
    
    return (ambient + diffuse * shadow) * albedo;
}

