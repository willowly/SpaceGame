#pragma once

#include <interface/interface.hpp>
#include "item/item-stack.hpp"
#include "text-widget.hpp"
#include "interface/widget.hpp"


class ItemSlotWidget : public Widget {

    public:
        Sprite sprite;
        Sprite barSprite;
        Color color;
        TextWidget* textWidget;
        vec2 size = vec2(60);

        float barWidth = 6.0; // the bottom bar to (normally) show durability
        float padding = 2.0;

        bool draw(DrawContext context,vec2 position,std::optional<ItemStack>& stackOpt) {
            return draw(context,Rect(position,size),stackOpt);
        }

        bool draw(DrawContext context,Rect rect,std::optional<ItemStack>& stackOpt) {
            if(stackOpt) {
                return draw(context,rect,stackOpt.value());
            } else {
                return drawEmpty(context,rect);
            }
        }

        bool drawEmpty(DrawContext context,vec2 position) {
            drawEmpty(context,Rect(position,size));
        }

        bool drawEmpty(DrawContext context,Rect rect) {
            context.drawRect(rect,sprite,color);
            if(context.mouseInside(rect)) {
                return true;
            }
            return false;
        }

        bool draw(DrawContext context,vec2 position,ItemStack& stack) {
            return draw(context,Rect(position,size),stack);
        }
    
        // returns if mouse inside
        bool draw(DrawContext context,Rect rect,ItemStack& stack) {

            bool mouseInside = drawEmpty(context,rect);
            if(stack.item == nullptr || stack.amount == 0) {
                return context.mouseInside(rect);
            }
            rect = Rect::anchored(Rect::centered(rect.size-padding),rect,vec2(0.5f,0.5f));
            context.drawRect(rect,stack.item->getIcon());

            if(stack.amount > 1 && textWidget != nullptr) {
                TextDisplaySettings settings;
                settings.pivot = vec2(1,1);
                textWidget->draw(context,rect.bottomRight(), std::to_string((int)stack.amount),settings);
            }

            
            auto display = stack.item->getItemDisplay(stack);
            if(display.bar) {
                context.drawRect(Rect(rect.position+vec2(0,rect.size.y-barWidth),vec2(rect.size.x*display.barPercent,barWidth)),barSprite,Color::red);
            }

            if(mouseInside) {
                context.setTooltip(stack);
            }

            return context.mouseInside(rect);
        }

        string getTypeName() override {
            return "item_slot_widget";
        }

};