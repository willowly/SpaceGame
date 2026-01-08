#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"


class CockpitBlock : public Block {
    public:

        CockpitBlock() : Block() {

        }


        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            character.ride(construction,position,Construction::getRotationFromFacing(state.facing));
        }
};