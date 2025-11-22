#pragma once

#include "block-actor.hpp"

class FurnaceBlockActor : public BlockActor {

    float fuel = 10;

    public:
        virtual void step(World* world,Construction* construction,ivec3 location,float dt) {
            if(fuel > 0) {
                fuel -= dt;
                std::cout << StringHelper::toString(location) << "fuel: " << fuel << std::endl;
            }
        }

};