#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "helper/block-helper.hpp"


class ConnectedBlock : public Block {
    public:

    
        ConnectedBlock() : Block() {
            solid = true;
        }

        TextureID texture = 0;

        int connectionType = 0; // :shrug: idk

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockHelper::addConnectedBlock(construction,meshData,position,texture);
        }

        string getTypeName() override {
            return "connected";
        }
};