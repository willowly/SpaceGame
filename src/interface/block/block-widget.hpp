
#pragma once

#include "interface/interface.hpp"
#include "helper/generic-storage.hpp"
#include "interface/widget.hpp"

template<typename BlockType>
class BlockWidget : public Widget {

    public:

        


    virtual void draw(DrawContext context,Character& user,BlockType& block,BlockStorage& storage,BlockState& state) = 0;

    
};