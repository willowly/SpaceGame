
#pragma once

#include "interface/interface.hpp"
#include "interface/widget.hpp"

template<typename ActorType>
class ActorWidget : public Widget{

    public:

        


    virtual void draw(DrawContext context,ActorType& actor) = 0;

    
};