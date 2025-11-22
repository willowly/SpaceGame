#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "text-widget.hpp"
#include "item-slot-widget.hpp"

class InventoryWidget {
    
    public:
        Sprite solid;
        Character* player;
        Font* font;
        vec2 size = vec2(80,50);
        float padding = 3;
        vec2 slotSize = vec2(10,10);
        float spacing = 2;

        ItemSlotWidget itemSlot;

        TextWidget tooltipTextTitle;

        void draw(DrawContext context) {

            Rect screen = context.getScreenSize();

            auto background = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = Rect::anchored(Rect::centered(size),screen,vec2(0.5,0.5));
            context.drawRect(mainPanel,background,solid);


            float width = 1.5;
            float ratio = 1.5;
            float padding = 0.1f;

            


            int i = 0;
            // int column = 0;
            // int columnMax = 5;
            Rect item = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));

            ItemStack* selectedItem = nullptr;
            Recipe* selectedRecipe = nullptr;
            for (auto stack : player->inventory.getItems())
            {
                if(stack != nullptr) {
                    if(itemSlot.draw(context,item,*stack)) {
                        selectedItem = stack;
                    }
                }
                
                item.position.x += slotSize.x + spacing;
                
            }

            item = Rect::anchored(Rect::withPivot(vec2(padding,-padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));

            for (auto& recipe : player->recipes)
            {
                context.drawRect(item,slots,solid);
                context.drawRect(item,Color::white,recipe->result.item->getIcon());
                if(context.mouseInside(item)) {
                    selectedRecipe = recipe;
                }

                if(recipe->result.amount > 1) {
                    std::string str = "" + std::to_string((int)recipe->result.amount);

                    vec2 position = vec2(0.0f);
                    std::reverse(str.begin(),str.end());
                    for(char c : str) {
                        context.drawRect(Rect::anchored(Rect::withPivot(position,vec2(width,width*ratio),vec2(1,1)),item,vec2(1,1)),Color::white,font->getSprite(c));
                        position.x -= width + padding;
                        
                    }
                }
                
                item.position.x -= slotSize.x + spacing;
                
            }

            if(selectedRecipe != nullptr) {
                drawTooltip(context,*selectedRecipe);
                if(context.mouseLeftClicked()) {
                    player->inventory.tryCraft(*selectedRecipe);
                }
                
            } else {
                if(selectedItem != nullptr) {
                    drawTooltip(context,*selectedItem);
                    for(int i = 0; i <= 9; i++)
                    if(context.getInput().getKeyPressed(GLFW_KEY_1 + i)) {
                        player->setToolbar(i,selectedItem->item);
                    }
                }
            }
            
            
        }

        void drawTooltip(DrawContext context,ItemStack& stack) {
            
            Rect tooltip = Rect(context.getMousePosition()+vec2(3,3),vec2(40,7));
            tooltip.size.x = tooltipTextTitle.getSize(stack.item->name).x + 4.0f;
            context.drawRect(tooltip,Color(0.05,0.05,0.05),solid);

            tooltipTextTitle.draw(context,tooltip.position + vec2(2.0f,2.0f),stack.item->name);
        }

        void drawTooltip(DrawContext context,Recipe& recipe) {
            
            Rect tooltip = Rect(context.getMousePosition()+vec2(3,3),vec2(40,7));

            string text = "CRAFT " + recipe.result.item->name;

            tooltip.size.x = tooltipTextTitle.getSize(text).x + 4.0f;
            context.drawRect(tooltip,Color(0.05,0.05,0.05),solid);

            tooltipTextTitle.draw(context,tooltip.position + vec2(2.0f,2.0f),text);
        }
};