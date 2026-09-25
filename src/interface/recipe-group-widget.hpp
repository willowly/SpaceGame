#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"

class RecipeGroupWidget : public Widget {
    
    public:
        Sprite backgroundSprite;
        Color backgroundColor;
        float margin = 5;
        float spacing = 2;
        int columns = 8;

        RecipeSlotWidget* recipeSlot;

        Recipe* draw(DrawContext context,Rect rect,std::vector<Recipe*>& recipes) {

            if(recipeSlot == nullptr) {
                Debug::warn("recipe slot is null (recipe group widget)");
                return nullptr;
            }

            Rect screen = context.getScreenSize();
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = rect;
            context.drawRect(mainPanel,backgroundSprite,backgroundColor);
            
            ItemSlotInteractOptions interactOptions;

            auto slotPosition = mainPanel.position;
            slotPosition += vec2(margin);

            Recipe* clickedRecipe = nullptr;

            bool hoveringPanel = context.mouseInside(mainPanel);
            int column = 0;
            for (auto recipe : recipes)
            {
                if(recipe == nullptr) continue;

                if(recipeSlot->draw(context,slotPosition,*recipe)) {
                    if(context.mouseLeftClicked()) {
                        clickedRecipe = recipe;
                    }
                }

                slotPosition.x += (recipeSlot->size.x + spacing);
                column++;
                if(column == columns) {
                    slotPosition.x = mainPanel.topLeft().x + margin;
                    slotPosition.y += (recipeSlot->size.y + spacing);
                    column = 0;
                }
            }

            return clickedRecipe;
            
            
        }

        string getTypeName() {
            return "recipe_group_widget";
        }
};