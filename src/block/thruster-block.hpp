#pragma once

#include "block.hpp"
#include "actor/construction.hpp"

#include "helper/block-helper.hpp"


class ThrusterBlock : public Block {
    public:

    
        ThrusterBlock() : Block() {
            
        }

        // ints
        static const int FACING_VAR = 0;

        Mesh<Vertex>* mesh;
        TextureID texture;
        float force; //for now this is always backwards
        float sideForce; //for now this is always the 4 directions parrellel to the back

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            storage.setFacing(FACING_VAR,facing);
            addThrustForces(construction,facing);
            return storage;
        }

        void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) {
            auto facing = storage.getFacing(FACING_VAR);
            addThrustForces(construction,facing);
        }

        void addThrustForces(Construction* construction,BlockFacing facing) {
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::UP,facing))] += sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::DOWN,facing))] += sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::RIGHT,facing))] += sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::LEFT,facing))] += sideForce;
            construction->thrustForces[static_cast<int>(facing)] += force;
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockHelper::addMesh(meshData,position,storage.getFacing(FACING_VAR),mesh,texture);
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) {

            auto facing = storage.getFacing(FACING_VAR);
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::UP,facing))] -= sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::DOWN,facing))] -= sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::RIGHT,facing))] -= sideForce;
            construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::LEFT,facing))] -= sideForce;
            construction->thrustForces[static_cast<int>(facing)] -= force;
        }
};