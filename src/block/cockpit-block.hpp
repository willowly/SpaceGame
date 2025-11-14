#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"


class CockpitBlock : public Block {
    public:

        CockpitBlock(Mesh* model,Material material) : Block(model,material) {

        }

        CockpitBlock() : CockpitBlock(nullptr,Material::none) {}

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            character.ride(construction,position,Construction::getRotationFromFacing(state.facing));
        }
};