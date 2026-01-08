layout(binding = 0) uniform SceneData {
    mat4 view;
    mat4 proj;
    mat4 screen;
    vec3 viewDir;
} sceneData[];