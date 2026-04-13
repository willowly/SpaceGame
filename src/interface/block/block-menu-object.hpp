#pragma once

#include "interface/menu-object.hpp"
#include "actor/construction.hpp"
#include "block-widget.hpp"
#include "glm/glm.hpp"

using glm::ivec3;

template<typename BlockType>
class BlockMenuObject : public MenuObject {

    public:

        Construction* construction;
        ivec3 location = {};
        BlockWidget<BlockType>& widget;

        BlockMenuObject(Construction* construction,ivec3 location,BlockWidget<BlockType>& widget) : construction(construction), location(location), widget(widget) {}

        void drawMenu(DrawContext context,Character& user) {
            Debug::addTrace("blockmenuobj");
            if(construction == nullptr) {
                Debug::warn("construction is null");
                Debug::subtractTrace();
                return;
            }
            
            
            auto entry = construction->getBlock(location).storage;
            BlockType* block = dynamic_cast<BlockType*>(entry.block);
            if(block == nullptr) {
                Debug::warn("block is null");
                Debug::subtractTrace();
                return;
            }
            auto storage = entry.storage;
            widget.draw(context,user,*block,storage,state);
            Debug::subtractTrace();
        }

    



};