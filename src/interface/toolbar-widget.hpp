#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "item-slot-widget.hpp"

class ToolbarWidget {
    
    public:
        Sprite solidSprite;
        Character* player;
        ItemSlotWidget itemSlot;

        void draw(DrawContext context) {

            Rect screen = context.getScreenSize();

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            for (int i = 0; i < 9; i++)
            {
                auto rect = Rect((i*10)-50,-11,10,10);
                rect = Rect::anchored(rect,screen,vec2(0.5,1));
                context.drawRect(rect,player->selectedTool == i ? selected : unselected,solidSprite);

                Item* tool = player->toolbar[i];
                ItemStack* stack = player->inventory.getStack(tool);
                if(stack != nullptr) {
                    if(itemSlot.draw(context,rect,*stack)) {
                        if(context.mouseLeftClicked()) {
                            player->setToolbar(i,nullptr);
                        }
                    }
                }
            }
            
        }
};