#pragma once

#include "block.hpp"
#include "actor/construction.hpp"


class ThrusterBlock : public Block {
    public:

    
        ThrusterBlock(Mesh* model,Material material,float force) : Block(model,material), force(force) {
            
        }
        ThrusterBlock() : ThrusterBlock(nullptr,Material::none,0) {}

        float force;

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            construction->thrustForces[state.facing] += force;
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {
            construction->thrustForces[state.facing] -= force;
        }
};