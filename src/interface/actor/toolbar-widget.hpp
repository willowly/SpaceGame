#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/item-slot-widget.hpp"
#include "interface/widget.hpp"

class ToolbarWidget : public Widget {
    
    public:
        Sprite solidSprite;
        ItemSlotWidget* itemSlot;

        void draw(DrawContext context,Character& player) {

            Rect screen = context.getScreenSize();

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            for (int i = 0; i < 9; i++)
            {
                auto rect = Rect((i*12)-50,-11,12,12);
                rect = Rect::anchored(rect,screen,vec2(0.5,1));
                context.drawRect(rect,player.selectedTool == i ? selected : unselected,solidSprite);
                if(context.mouseInside(rect)) {
                    if(context.mouseLeftClicked()) {
                        player.setToolbar(i,player.replaceCursor(player.toolbar[i]));
                    }
                }

                auto& stack = player.toolbar[i];
                if(!stack.isEmpty()) {
                    if(itemSlot == nullptr) {
                        Debug::warn("Itemslot Widget is null!");
                        break;
                    }
                    itemSlot->draw(context,rect,stack);
                }
            }
            
        }
};