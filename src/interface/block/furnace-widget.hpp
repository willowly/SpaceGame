#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"
#include "block/crafter-block.hpp"
#include "block-widget.hpp"
#include "interface/item-slot-interact-options.hpp"

class FurnaceWidget : public BlockWidget<CrafterBlock> {
    
    public:
        Sprite solid;
        Font* font = nullptr;
        vec2 size = vec2(380,80);
        float padding = 3;
        vec2 slotSize = vec2(60,60);
        float spacing = 2;
        float barWidth = 60; 

        ItemSlotWidget* itemSlot = {};
        RecipeGroupWidget* recipeGroup = {};

        TextWidget* tooltipTextTitle = {};

        void draw(DrawContext context,Construction* construction,Character& user,CrafterBlock& furnace,BlockStorage& storage) {


            if(itemSlot == nullptr) {
                Debug::warn("itemSlot is null");
                return;
            }
            if(recipeGroup == nullptr) {
                Debug::warn("recipe group is null");
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
            std::vector<ItemStack> inputStacks;
            for (size_t i = 0; i < furnace.maxIngredients; i++)
            {
                inputStacks.push_back(storage.getStack(furnace.INPUTSTACK_VAR+i));
            }
            
            auto outputStack = storage.getStack(furnace.OUTPUTSTACK_VAR);
            auto fuelStack = furnace.fuelBurner.getFuelStack(storage);
            auto currentRecipe = storage.getPointer<Recipe>(furnace.CURRENTRECIPE_VAR);
            float timer = storage.getFloat(furnace.TIMER_VAR);
            float fuel = furnace.fuelBurner.getFuel(storage);
            float fuelMax = furnace.fuelBurner.getFuelMax(storage);

            auto slotRect = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));
            for (size_t i = 0; i < furnace.maxIngredients; i++)
            {
                // input slots
                if(itemSlot->drawAndInteract(context,slotRect,inputStacks[i],user)) {
                    furnace.trySetMatchingRecipe(currentRecipe,storage);
                }
                slotRect.position.x += slotRect.size.x + spacing;
            }
            slotRect.position.x -= slotRect.size.x + spacing;
            
            auto barRect = Rect(slotRect.topRight(),vec2(0.0f));

            // output slot
            slotRect = Rect::anchored(Rect::withPivot(vec2(-padding,padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));
            ItemSlotInteractOptions options;
            options.allowInsert = false;
            itemSlot->drawAndInteract(context,slotRect,outputStack,user,options);

            // recipe bar
            barRect.size = slotRect.bottomLeft() - barRect.position;
            barRect = Rect::anchored(Rect::withPivot(vec2(barRect.size.x - padding*2.0f,barWidth),vec2(0.5,0.5)),barRect,vec2(0.5,0.5f));
            context.drawRect(barRect,solid,slots); //background
            if(currentRecipe != nullptr) {
                auto progress = timer/currentRecipe->time;
                progress = fmin(fmax(progress,0),1);
                auto barRectFront = Rect::anchored(Rect::withPivot(vec2(barRect.size.x*progress,barRect.size.y),vec2(0,0.5)),barRect,vec2(0,0.5));
                context.drawRect(barRectFront,solid,Color::red); //foreground
            }


            Rect fuelRect = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));
            fuelRect.position.y += slotSize.y + spacing;

            float progress = 0;
            if(!furnace.electric) {
                //fuel slot
                
                itemSlot->drawAndInteract(context,fuelRect,fuelStack,user);
                progress = fuel/fuelMax;
            } else {
                auto network = construction->getNetwork(0);
                progress = network.getCurrentCharge() / network.getMaxCharge();
            }

            // fuel bar
            auto fuelBarRect = barRect;
            fuelBarRect.position.y += slotSize.y + spacing;
            context.drawRect(fuelBarRect,solid,slots); //background
            progress = fmin(fmax(progress,0),1);
            fuelBarRect = Rect::anchored(Rect::withPivot(vec2(fuelBarRect.size.x*progress,fuelBarRect.size.y),vec2(0,0.5)),fuelBarRect,vec2(0,0.5));
            context.drawRect(fuelBarRect,solid,Color::red); //foreground
             
            for (size_t i = 0; i < furnace.maxIngredients; i++)
            {
                storage.setStack(furnace.INPUTSTACK_VAR+i,inputStacks[i]); // try start craft overrides it
            }

            
            
            storage.setPointer<Recipe>(furnace.CURRENTRECIPE_VAR,currentRecipe);
            storage.setStack(furnace.OUTPUTSTACK_VAR,outputStack);
            furnace.fuelBurner.setFuelStack(storage,fuelStack);

            if(furnace.allowManual) {
                context.drawRect(fuelRect,solid,Color::green);
                if(context.mouseInside(fuelRect)) {
                    if(context.mouseLeftClicked()) {
                        furnace.progressManual(storage);
                    }
                }
            }

            auto clickedRecipe = recipeGroup->draw(context,Rect::anchored(Rect::withPivot(vec2(mainPanel.size.x,mainPanel.size.y*0.5f),vec2(0.5,0)),mainPanel,vec2(0.5,0.5)),furnace.recipes);
            if(clickedRecipe != nullptr) {
                furnace.tryStartCraft(*clickedRecipe,user,storage);
            }

            //storage.setStack(furnace.FUELSTACK_VAR,fuelStack);
            
            
        }

        string getTypeName() override {
            return "furnace_widget";
        }
};