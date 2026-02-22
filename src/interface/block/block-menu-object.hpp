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
            
            BlockType* block = dynamic_cast<BlockType*>(construction->getBlock(location).first);
            BlockState state = construction->getBlock(location).second;
            if(block == nullptr) {
                Debug::warn("construction is null");
                Debug::subtractTrace();
                return;
            }
            auto storage = construction->getStorage(location);
            if(storage == nullptr) {
                Debug::warn("storage is null");
                Debug::subtractTrace();
                return;
            }

            widget.draw(context,user,*block,*storage,state);
        }

    



};