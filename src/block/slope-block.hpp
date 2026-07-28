#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "helper/block-helper.hpp"


class SlopedBlock : public Block {
    public:

    
        SlopedBlock() : Block() {
            solid = true;
        }
        
        TextureID texture = 0;

        int connectionType = 0; // :shrug: idk

        // ints
        static const int ORIENTATION_VAR = 0;

        BlockStorage onPlace(Construction *construction, ivec3 position, BlockPlaceInfo placeInfo) override
        {
            BlockStorage storage;
            storage.setInt(ORIENTATION_VAR, 0);

            return storage;

        }

        void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) override {
            auto orientation = storage.getInt(ORIENTATION_VAR, 0);
            
            BlockHelper::addSlopedBlock(construction,meshData,position,orientation,texture);
        }

        string getTypeName() override {
            return "sloped";
        }
};