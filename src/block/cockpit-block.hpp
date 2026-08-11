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
        static const int ROTATION_VAR = 1;

        Mesh<Vertex>* mesh = {};
        TextureID texture = {};

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            storage.setFacing(FACING_VAR,facing);
            vec2 up = glm::inverse(BlockHelper::getRotationFromFacing(facing)) * placeInfo.upDir;
            int rotation = BlockHelper::getRotationIndexFromVector(up);

            storage.setInt(ROTATION_VAR,rotation);
            return storage;
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            int rotationIndex = storage.getInt(ROTATION_VAR);
            BlockHelper::addMesh(meshData,position,facing,rotationIndex,mesh,texture);
        }


        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            int rotationIndex = storage.getInt(ROTATION_VAR);
            character.ride(construction,position,BlockHelper::getRotationFromFacing(facing,rotationIndex));
        }

        string getTypeName() override {
            return "cockpit";
        }
};