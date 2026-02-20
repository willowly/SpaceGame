#ifndef SCENE_DATA
#define SCENE_DATA
#extension GL_EXT_scalar_block_layout : require

layout(scalar,binding = 0) uniform SceneData {
    mat4 view;
    mat4 proj;
    mat4 screen;
    vec3 viewDir;
    vec3 mainLightColor;
    vec3 mainLightDirection;
    vec3 ambientLightColor;
} sceneData[];

#endif //__SCENE_DATA__