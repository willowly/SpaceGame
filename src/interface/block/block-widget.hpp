
#pragma once

#include "interface/interface.hpp"
#include "helper/generic-storage.hpp"
#include "interface/widget.hpp"

template<typename BlockType>
class BlockWidget : public Object {

    public:

        


    virtual void draw(DrawContext context,Character& user,BlockType& block,BlockStorage& storage) = 0;

    
};