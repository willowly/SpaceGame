#pragma once

#include <interface/interface.hpp>
#include "item/item-stack.hpp"
#include "text-widget.hpp"
#include "interface/widget.hpp"


class RecipeSlotWidget : public Widget {

    public:
        Sprite sprite;
        Color color;
        TextWidget* text;
        vec2 size = vec2(60);

        float padding = 2.0;


        bool draw(DrawContext context,vec2 position,Recipe& recipe) {
            return draw(context,Rect(position,size),recipe);
        }
    
        // returns if mouse inside
        bool draw(DrawContext context,Rect rect,Recipe& recipe) {

            context.drawRect(rect,sprite,color);
            if(recipe.result.isEmpty()) {
                return context.mouseInside(rect);
            }
            rect = Rect::anchored(Rect::centered(rect.size-padding),rect,vec2(0.5f,0.5f));
            context.drawRect(rect,recipe.result.item->getIcon());

            bool mouseInside = context.mouseInside(rect);

            if(mouseInside) {
                context.setTooltip(&recipe);
            }

            return mouseInside;
        }

        string getTypeName() override {
            return "recipe_slot_widget";
        }

};