#pragma once

#include <interface/interface.hpp>
#include "item/item-stack.hpp"
#include "text-widget.hpp"


class ItemSlotWidget {

    public:
        Sprite sprite;
        Color color;
        Font* font;

    
        // returns if mouse inside
        bool draw(DrawContext context,Rect rect,ItemStack& stack) {

            context.drawRect(rect,color,sprite);
            context.drawRect(rect,Color::white,stack.item->getIcon());

            if(stack.amount) {
                float width = 2;
                float ratio = 1.5;
                float padding = 0.1f;

                if(stack.amount > 1) {
                    std::string str = std::to_string((int)stack.amount);

                    vec2 position = vec2(0.0f);
                    std::reverse(str.begin(),str.end());
                    for(char c : str) {
                        context.drawRect(Rect::anchored(Rect::withPivot(position,vec2(width,width*ratio),vec2(1,1)),rect,vec2(1,1)),Color::white,font->getSprite(c));
                        position.x -= width + padding;
                        
                    }
                }
            }

            if(context.mouseInside(rect)) {
                return true;
            }
            return false;
        }

};