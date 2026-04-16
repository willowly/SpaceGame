#pragma once

#include "terrain-vertex.hpp"
#include "voxel-data.hpp"

struct TerrainMaterial {
    LitMaterialData terrainTypes[8];
};

struct TerrainType {
    Item* item;
    TextureID texture;
};

struct GenerationSettings {
    float noiseScale = 100;
    float radius = 50;
    float noiseFactor = 30;
    int noiseOctaves = 1;
    float noiseGain = 0.5f;
    float noiseLacunarity = 2.0;
    TerrainType stoneType;
    TerrainType oreType;

    struct OreSettings {
        TerrainType type;
        
    };

};

struct TerraformResults {

    std::vector<ItemStack> items;

    void addItem(ItemStack giveStack) {
        for (auto&& stack : items)
        {
            if(stack.item == giveStack.item) {
                stack.amount += giveStack.amount;
                return;
            }
        }
        items.push_back(giveStack);
    }
};