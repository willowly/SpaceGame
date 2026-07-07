
#pragma once

#include "interface/interface.hpp"
#include "interface/widget.hpp"

template<typename ActorType>
class ActorWidget : public Object {

    public:

        


    virtual void draw(DrawContext context,ActorType& actor) = 0;

    
};