#pragma once

#include "block.hpp"
#include "actor/construction.hpp"

#include "helper/block-helper.hpp"


class ThrusterBlock : public Block {
    public:

    
        ThrusterBlock() : Block() {
            
        }

        Mesh<Vertex>* mesh;
        TextureID texture;
        float force; //for now this is always backwards
        float sideForce; //for now this is always the 4 directions parrellel to the back

        virtual BlockState onPlace(Construction* construction,ivec3 position,BlockFacing facing) {

            
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::UP,facing)] += sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::DOWN,facing)] += sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::RIGHT,facing)] += sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::LEFT,facing)] += sideForce;
            construction->thrustForces[facing] += force;
            return BlockState::encode(facing);
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockState& state) {
            BlockHelper::addMesh(meshData,position,state.asFacing(),mesh,texture);
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {

            auto facing = state.asFacing();
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::UP,facing)] -= sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::DOWN,facing)] -= sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::RIGHT,facing)] -= sideForce;
            construction->thrustForces[BlockHelper::rotateFacingByFacing(BlockFacing::LEFT,facing)] -= sideForce;
            construction->thrustForces[facing] -= force;
        }
};