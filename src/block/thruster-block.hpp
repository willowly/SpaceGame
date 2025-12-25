#pragma once

#include "block.hpp"
#include "actor/construction.hpp"


class ThrusterBlock : public Block {
    public:

    
        ThrusterBlock() : Block() {
            
        }

        float force;

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            construction->thrustForces[state.facing] += force;
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {
            construction->thrustForces[state.facing] -= force;
        }
};