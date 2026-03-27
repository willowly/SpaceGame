#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/item-slot-widget.hpp"
#include "interface/widget.hpp"

class ToolbarWidget : public Widget {
    
    public:
        Sprite itemSlotSprite;
        Sprite sprite;
        Sprite selectorSprite;
        ItemSlotWidget* itemSlot;
        float slotSize = 10;
        float slotGap = 2;
        float slotHeight = 100;
        float selectorSize = 12;

        void draw(DrawContext context,Character& player) {

            Rect screen = context.getScreenSize();

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);

            context.drawRect(Rect::anchored(Rect::withPivot(vec2(0,-10),vec2(800,160),vec2(0.5f,1.0f)),screen,vec2(0.5,1.0f)),sprite);
            for (int i = 0; i < 9; i++)
            {
                auto rect = Rect((i*(slotSize+slotGap))-((slotSize+slotGap)*4.5f)+slotGap/2,-slotHeight,slotSize,slotSize);
                rect = Rect::anchored(rect,screen,vec2(0.5,1));
                context.drawRect(rect,itemSlotSprite);
                if(context.mouseInside(rect)) {
                    if(context.mouseLeftClicked()) {
                        player.setToolbar(i,player.replaceCursor(player.toolbar[i]));
                    }
                }
                
                if(player.selectedTool == i) {
                    context.drawRect(Rect::centered(rect.center(),vec2(selectorSize)),selectorSprite);
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