#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"

class InventoryWidget : public Widget{
    
    public:
        Sprite backgroundSprite;
        Font* font;
        vec2 size = vec2(150,40);
        float padding = 3;
        vec2 slotSize = vec2(10,10);
        float spacing = 2;

        ItemSlotWidget* itemSlot;

        TextWidget* tooltipTextTitle;
        ItemSlotWidget* recipeSlot;

        void draw(DrawContext context,Character& player) {

            Rect screen = context.getScreenSize();

            auto background = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = Rect::anchored(Rect::withPivot(vec2(0,3),size,vec2(0.5f,0)),screen,vec2(0.5,0.5));
            context.drawRect(mainPanel,background,backgroundSprite);


            float width = 1.5;
            float ratio = 1.5;
            float padding = 0.1f;

            


            Rect item = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));

            ItemStack* selectedItem = nullptr;
            Recipe* selectedRecipe = nullptr;
            bool itemHover = false;
            for (auto stack : player.inventory.getItems())
            {
                if(stack != nullptr) {
                    if(itemSlot->draw(context,item,*stack)) {
                        itemHover = true;
                        if(context.mouseLeftClicked()) {
                            auto droppedStack = player.replaceCursor(*stack);
                            player.inventory.take(*stack);
                            player.inventory.give(droppedStack);
                        } else {
                            selectedItem = stack; //if we pick it up, its no longer selected
                        }
                    }
                }
                
                item.position.x += slotSize.x + spacing;
                
            }

            if(!itemHover && context.mouseInside(mainPanel)) {
                if(context.mouseLeftClicked()) {
                    auto droppedStack = player.dropCursor();
                    player.inventory.give(droppedStack);
                }
            }

            item = Rect::anchored(Rect::withPivot(vec2(padding,-padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));

            for (auto recipe : player.recipes)
            {
                if(recipe == nullptr) {
                    Debug::warn("null recipe in player");
                    continue;
                }
                if(recipeSlot->draw(context,item,recipe->result)) {
                    selectedRecipe = recipe;
                }

                if(player.currentRecipe == recipe) {
                    float progressPercent = player.recipeTimer/recipe->time;
                    //context.drawRect(item,slots,solid);
                    Rect fill(item.position + vec2(0,(1-progressPercent)*item.size.y),vec2(item.size.x,item.size.y * progressPercent));
                    context.drawRect(fill,Color(1,1,1,0.2),backgroundSprite);
                }
                
                item.position.x -= slotSize.x + spacing;
                
            }

            if(selectedRecipe != nullptr) {
                drawTooltip(context,*selectedRecipe);
                if(context.mouseLeftClicked()) {
                    player.startCraft(*selectedRecipe);
                }
                
            } else {
                if(selectedItem != nullptr) {
                    drawTooltip(context,*selectedItem);
                    for(int i = 0; i <= 9; i++) {
                        if(context.getInput().getKeyPressed(GLFW_KEY_1 + i)) {
                            player.setToolbar(i,*selectedItem);
                            player.inventory.take(*selectedItem);
                        }
                    }
                }
            }
            
            
        }

        void drawTooltip(DrawContext context,ItemStack& stack) {
            
            Rect tooltip = Rect(context.getMousePosition()+vec2(3,3),vec2(40,7));
            tooltip.size.x = tooltipTextTitle->getSize(stack.item->name).x + 4.0f;
            context.drawRect(tooltip,Color(0.05,0.05,0.05),backgroundSprite);

            tooltipTextTitle->draw(context,tooltip.position + vec2(2.0f,2.0f),stack.item->name);
        }

        void drawTooltip(DrawContext context,Recipe& recipe) {
            
            Rect tooltip = Rect(context.getMousePosition()+vec2(3,3),vec2(40,7));

            string text = "CRAFT " + recipe.result.item->name;

            tooltip.size.x = tooltipTextTitle->getSize(text).x + 4.0f;
            context.drawRect(tooltip,Color(0.05,0.05,0.05),backgroundSprite);

            tooltipTextTitle->draw(context,tooltip.position + vec2(2.0f,2.0f),text);
        }
};