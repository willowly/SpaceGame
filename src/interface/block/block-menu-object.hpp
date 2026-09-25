#pragma once

#include "interface/menu-object.hpp"
#include "actor/construction.hpp"
#include "block-widget.hpp"
#include "glm/glm.hpp"

using glm::ivec3;

template<typename BlockType>
class BlockMenuObject : public MenuObject {

    public:

        ActorID constructionID;
        ivec3 location = {};
        BlockWidget<BlockType>& widget;

        BlockMenuObject(ActorID constructionID,ivec3 location,BlockWidget<BlockType>& widget) : constructionID(constructionID), location(location), widget(widget) {}

        void drawMenu(DrawContext context,World& world,Character& user) {
            Debug::addTrace("blockmenuobj");

            auto construction = world.getActor<Construction>(constructionID);
            if(construction == Invalid_ActorID) {
                Debug::warn("construction is null");
                Debug::subtractTrace();
                return;
            }
            if(construction->destroyed) {
                return;
            }
            
            
            auto& entry = construction->getBlock(location);
            BlockType* block = dynamic_cast<BlockType*>(entry.block);
            if(block == nullptr) {
                Debug::warn("block is null");
                Debug::subtractTrace();
                return;
            }
            auto& storage = entry.storage;
            widget.draw(context,construction,user,*block,storage);
            Debug::subtractTrace();
        }

    



};