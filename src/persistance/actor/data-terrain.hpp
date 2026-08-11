#pragma once
#include "persistance/data-generic.hpp"
#include "persistance/item/data-item-stack.hpp"
#include "persistance/actor/data-actor.hpp"
#include "actor/terrain/voxel-data.hpp"
#include "cista.h"

struct data_TerrainChunk {
    data_ivec3 location = {};
    cista::raw::vector<VoxelData> terrainData;
    cista::raw::vector<cista::raw::string> terrainTypes;
};


struct data_TerrainLayer {
    cista::raw::vector<data_TerrainChunk> chunks;
};

struct data_Terrain {

    data_Actor actor;
    unsigned int seed = {};
    cista::raw::string terrainSettings;

    cista::raw::vector<data_TerrainLayer> chunkLayers;

};