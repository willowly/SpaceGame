#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"

class CockpitBlock : public Block {
    public:

        CockpitBlock() : Block() {

        }

        // ints
        static const int FACING_VAR = 0;

        Mesh<Vertex>* mesh;
        TextureID texture;

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            storage.setFacing(FACING_VAR,BlockHelper::getFacingFromVector(placeInfo.normal));
            return storage;
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            BlockHelper::addMesh(meshData,position,facing,mesh,texture);
        }


        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            character.ride(construction,position,BlockHelper::getRotationFromFacing(facing));
        }
};