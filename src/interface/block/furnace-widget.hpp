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
#include "interface/item-slot-interact-options.hpp"

class FurnaceWidget : public BlockWidget<FurnaceBlock> {
    
    public:
        Sprite solid;
        Font* font = nullptr;
        vec2 size = vec2(380,80);
        float padding = 3;
        vec2 slotSize = vec2(60,60);
        float spacing = 2;
        float barWidth = 60; 

        ItemSlotWidget* itemSlot = {};
        RecipeSlotWidget* recipeSlot = {};

        TextWidget* tooltipTextTitle = {};

        void draw(DrawContext context,Character& user,FurnaceBlock& furnace,BlockStorage& storage) {


            if(itemSlot == nullptr) {
                Debug::warn("itemSlot is null");
                return;
            }
            if(recipeSlot == nullptr) {
                Debug::warn("recipeSlot is null");
                return;
            }
            Rect screen = context.getScreenSize();

            auto backgroundColor = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = Rect::anchored(Rect::withPivot(vec2(padding,0),size,vec2(0,0.5)),screen,vec2(0.5,0.5));
            context.drawRect(mainPanel,solid,backgroundColor);


            // variables
            auto inputStack = storage.getStack(furnace.INPUTSTACK_VAR);
            auto outputStack = storage.getStack(furnace.OUTPUTSTACK_VAR);
            auto currentRecipe = storage.getPointer<Recipe>(furnace.CURRENTRECIPE_VAR);
            float timer = storage.getFloat(furnace.TIMER_VAR);

            ItemStack* selectedSlot = nullptr;

            // input slot
            auto slotRect = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));
            if(itemSlot->draw(context,slotRect,inputStack)) {
                selectedSlot = &inputStack;
                if(user.itemSlotHoverActions(context,inputStack)) {
                    furnace.trySetMatchingRecipe(currentRecipe,inputStack);
                }
            }
            
            auto barRect = Rect(slotRect.topRight(),vec2(0.0f));

            // output slot
            slotRect = Rect::anchored(Rect::withPivot(vec2(-padding,padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));
            if(itemSlot->draw(context,slotRect,outputStack)) {
                selectedSlot = &outputStack;
                ItemSlotInteractOptions options;
                options.allowInsert = false;
                user.itemSlotHoverActions(context,outputStack,options);
            }

            barRect.size = slotRect.bottomLeft() - barRect.position;
            barRect = Rect::anchored(Rect::withPivot(vec2(barRect.size.x - padding*2.0f,barWidth),vec2(0.5,0.5)),barRect,vec2(0.5,0.5f));
            context.drawRect(barRect,solid,slots); //background
            if(currentRecipe != nullptr) {
                auto progress = timer/currentRecipe->time;
                progress = fmin(fmax(progress,0),1);
                barRect = Rect::anchored(Rect::withPivot(vec2(barRect.size.x*progress,barRect.size.y),vec2(0,0.5)),barRect,vec2(0,0.5));
                context.drawRect(barRect,solid,Color::red); //foreground
            }

            Rect recipeRect = Rect::anchored(Rect(vec2(padding,padding),slotSize),mainPanel,vec2(0,0));
            recipeRect.position.y += slotSize.y + spacing;

            Recipe* selectedRecipe = nullptr;

            for (auto& recipe : furnace.recipes)
            {
                if(recipe == nullptr) {
                    Debug::warn("null recipe in furnace");
                    continue;
                }
                if(recipeSlot->draw(context,recipeRect,*recipe)) {
                    selectedRecipe = recipe;
                }
                
                recipeRect.position.x += slotSize.x + spacing;
                
            }



            if(selectedRecipe != nullptr) {
                if(context.mouseLeftClicked()) {
                    furnace.tryStartCraft(*selectedRecipe,user,storage);
                }
                return; //need to fix this,see comment below
                
            }

            // variables
            storage.setPointer<Recipe>(furnace.CURRENTRECIPE_VAR,currentRecipe);
            storage.setStack(furnace.INPUTSTACK_VAR,inputStack);
            storage.setStack(furnace.OUTPUTSTACK_VAR,outputStack);
            
            
        }

        string getTypeName() override {
            return "furnace_widget";
        }
};