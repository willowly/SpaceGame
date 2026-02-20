#ifndef SHADOW_MAPS
#define SHADOW_MAPS
#include "scene_data.hlsl"

layout(binding = 2) uniform sampler2D shadowMaps[];

float texelSize = 1.0f/10000.0f;

float getShadow(vec4 lightSpacePosition) {

    float bias = 0.001f;

    vec3 shadowTexCoord = ((lightSpacePosition.xyz/lightSpacePosition.w) + vec3(1.0f,1.0f,0.0))*vec3(0.5f,0.5f,1.0f); //we need to convert from NDC to texcoords. Z is fine as is
    //shadowTexCoord.x += 1.0f;
    if(shadowTexCoord.z < 0.0f || shadowTexCoord.z > 1.0f) return 1.0f;

    float shadow = 0.0;
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float depth = texture(shadowMaps[0],shadowTexCoord.xy + (vec2(x,y) * texelSize)).r;
            shadow += shadowTexCoord.z - bias < depth ? 1.0f : 0.0f;
        }
    }
    shadow /= 9.0f;
    return shadow;

}

vec4 getLightSpacePos(uint frameIndex,mat4 modelMatrix,vec3 inPosition) {
    return sceneData[frameIndex+2].proj * sceneData[frameIndex+2].view * modelMatrix * vec4(inPosition, 1.0);
}

#endif