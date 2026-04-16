#pragma once

#include "volk.h"

class Vulkan;
typedef VkDeviceAddress MaterialHandle;

class Material {
    friend Vulkan;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipeline shadowPipeline = VK_NULL_HANDLE;
    MaterialHandle data = 0;
    Material(VkPipeline pipeline, MaterialHandle data) : pipeline(pipeline), data(data) {}
    Material(VkPipeline pipeline,VkPipeline shadowPipeline, MaterialHandle data) : pipeline(pipeline), shadowPipeline(shadowPipeline), data(data) {}
    Material() {}
    public:
        static const Material none;
        bool isValid() {
            return data != 0;
        }
};

inline const Material Material::none = Material();