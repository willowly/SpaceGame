
#pragma once

#include "interface/interface.hpp"
#include "helper/generic-storage.hpp"

template<typename BlockType>
class BlockWidget {

    public:

        


    virtual void draw(DrawContext context,Character& user,BlockType& block) = 0;

    
};