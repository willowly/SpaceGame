#pragma once
#include "graphics/mesh.hpp"
#include <vector>
#include "glm/glm.hpp"
#include "item/item.hpp"
#include "item/item-stack.hpp"
#include "SimplexNoise.h"

using glm::vec3,glm::ivec4;

struct TerrainVertex {
    vec3 pos;
    vec3 normal;
    ivec4 textureID = ivec4(0);
    vec4 oreBlend = vec4(0.0);
    TerrainVertex(vec3 pos,ivec4 textureID,vec4 oreBlend) : pos(pos),textureID(textureID),oreBlend(oreBlend)  {}
    TerrainVertex(vec3 pos) : pos(pos)  {}

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(TerrainVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(TerrainVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(TerrainVertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SINT;
        attributeDescriptions[2].offset = offsetof(TerrainVertex, textureID);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(TerrainVertex, oreBlend);

        return attributeDescriptions;
    }
};

struct TerrainMaterial {
    LitMaterialData terrainTypes[8];
};

struct TerrainType {
    Item* item;
    TextureID texture;
};

struct GenerationSettings {
    float noiseScale = 100;
    float radius = 70;
    TerrainType stoneType;
    TerrainType oreType;
};

struct TerraformResults {

    std::vector<ItemStack> items;

    void addItem(ItemStack giveStack) {
        for (auto&& stack : items)
        {
            if(stack.item == giveStack.item) {
                stack.amount = giveStack.amount;
                return;
            }
        }
        items.push_back(giveStack);
    }
};