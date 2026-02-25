#pragma once

#include <interface/interface.hpp>
#include "item/item-stack.hpp"
#include "text-widget.hpp"
#include "interface/widget.hpp"


class ItemSlotWidget : public Widget {

    public:
        Sprite sprite;
        Color color;
        Font* font;

        float barWidth = 1.0; // the bottom bar to (normally) show durability
        float padding = 2.0;

        bool draw(DrawContext context,Rect rect,std::optional<ItemStack>& stackOpt) {
            if(stackOpt) {
                return draw(context,rect,stackOpt.value());
            } else {
                return drawEmpty(context,rect);
            }
        }

        bool drawEmpty(DrawContext context,Rect rect) {
            context.drawRect(rect,color,sprite);
            if(context.mouseInside(rect)) {
                return true;
            }
            return false;
        }
    
        // returns if mouse inside
        bool draw(DrawContext context,Rect rect,ItemStack& stack) {

            drawEmpty(context,rect);
            if(stack.item == nullptr || stack.amount == 0) {
                return context.mouseInside(rect);
            }
            rect = Rect::anchored(Rect::centered(rect.size-padding),rect,vec2(0.5f,0.5f));
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

            
            auto display = stack.item->getItemDisplay(stack);
            if(display.bar) {
                context.drawRect(Rect(rect.position+vec2(0,rect.size.y-barWidth),vec2(rect.size.x*display.barPercent,barWidth)),Color::red,sprite);
            }

            return context.mouseInside(rect);
        }

};