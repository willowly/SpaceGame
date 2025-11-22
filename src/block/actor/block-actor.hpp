#pragma once

#include "block/block.hpp"
#include "block/block-state.hpp"


class BlockActor {
    
    public:
        virtual void step(World* world,Construction* construction,ivec3 location,float dt) {

        }
};