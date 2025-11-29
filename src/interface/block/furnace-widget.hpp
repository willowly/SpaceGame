#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"
#include "block/furnace-block.hpp"
#include "block-widget.hpp"

class FurnaceWidget : public BlockWidget<FurnaceBlock> {
    
    public:
        Sprite solid;
        Font* font;
        vec2 size = vec2(80,20);
        float padding = 3;
        vec2 slotSize = vec2(10,10);
        float spacing = 2;

        ItemSlotWidget* itemSlot;

        TextWidget tooltipTextTitle;

        void draw(DrawContext context,Character& user,FurnaceBlock& furnace,BlockStorage& storage,BlockState& state) {

            Rect screen = context.getScreenSize();

            auto background = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = Rect::anchored(Rect::withPivot(vec2(0,-3),size,vec2(0.5,1)),screen,vec2(0.5,0.5));
            context.drawRect(mainPanel,background,solid);


            
            auto inputStack = storage.getStack(furnace.INPUTSTACK_VAR);
            auto outputStack = storage.getStack(furnace.OUTPUTSTACK_VAR);
            auto currentRecipe = storage.getPointer<Recipe>(furnace.CURRENTRECIPE_VAR);
            float timer = storage.getFloat(furnace.TIMER_VAR);

            ItemStack* selectedSlot = nullptr;

            auto slotRect = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));
            if(itemSlot->draw(context,slotRect,inputStack)) {
                selectedSlot = &inputStack;
            }

            auto barRect = Rect::withPivot(slotRect.topRight()+vec2(15-slotRect.size.x*0.5f,slotRect.size.y*0.5f),vec2(15.0-slotRect.size.x*0.5f,3.0),vec2(0.5f));

            context.drawRect(barRect,slots,solid);

            if(currentRecipe != nullptr) {
                auto progress = timer/currentRecipe->time;
                progress = fmin(fmax(progress,0),1);
                barRect = Rect::anchored(Rect::withPivot(vec2(barRect.size.x*progress,barRect.size.y),vec2(0,0.5)),barRect,vec2(0,0.5));
                context.drawRect(barRect,Color::red,solid);
            }

            slotRect.position += vec2(30,0);
            if(itemSlot->draw(context,slotRect,outputStack)) {
                selectedSlot = &outputStack;
            }

            Rect item = Rect::anchored(Rect::withPivot(vec2(-padding,padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));

            Recipe* selectedRecipe = nullptr;

            for (auto& recipe : furnace.recipes)
            {

                if(itemSlot->draw(context,item,recipe->result)) {
                    selectedRecipe = recipe;
                }
                
                item.position.x -= slotSize.x + spacing;
                
            }

            if(selectedRecipe != nullptr) {
                drawTooltip(context,*selectedRecipe);
                if(context.mouseLeftClicked()) {
                    furnace.tryStartCraft(*selectedRecipe,user,storage,state);
                }
                return; //need to fix this,see comment below
                
            } else {
                if(selectedSlot != nullptr && selectedSlot->item != nullptr) {
                    drawTooltip(context,*selectedSlot);
                    if(context.mouseLeftClicked()) {
                        user.inventory.give(*selectedSlot); //make this a call to the furnace :)
                        selectedSlot->clear();
                    }
                }
                
            }

            storage.setStack(furnace.INPUTSTACK_VAR,inputStack);
            storage.setStack(furnace.OUTPUTSTACK_VAR,outputStack);
            
            
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