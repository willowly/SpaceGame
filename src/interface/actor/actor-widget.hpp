
#pragma once

#include "interface/interface.hpp"

template<typename ActorType>
class ActorWidget {

    public:

        


    virtual void draw(DrawContext context,ActorType& actor) = 0;

    
};