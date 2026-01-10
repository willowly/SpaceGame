#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"

class CockpitBlock : public Block {
    public:

        CockpitBlock() : Block() {

        }

        Mesh<Vertex>* mesh;
        TextureID texture;

        virtual BlockState onPlace(Construction* construction,ivec3 position,BlockFacing facing) {
            std::cout << "facing " << facing << std::endl;
            return BlockState::encode(facing);
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockState& state) {
            BlockHelper::addMesh(meshData,position,state.asFacing(),mesh,texture);
        }


        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            character.ride(construction,position,BlockHelper::getRotationFromFacing(state.asFacing()));
        }
};