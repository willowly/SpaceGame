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
        vec2 size = vec2(80,50);
        float padding = 3;
        vec2 slotSize = vec2(10,10);
        float spacing = 2;

        ItemSlotWidget* itemSlot;

        TextWidget tooltipTextTitle;

        void draw(DrawContext context,Character& user,FurnaceBlock& furnace,GenericStorage& storage,BlockState& state) {

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

            
            auto inputStackOpt = storage.getStack(furnace.INPUTSTACK_VAR);

            auto inputRect = Rect::anchored(Rect(vec2(padding),slotSize),mainPanel,vec2(0,0));
            itemSlot->draw(context,inputRect,inputStackOpt);

            Rect item = Rect::anchored(Rect::withPivot(vec2(padding,-padding),slotSize,vec2(1,0)),mainPanel,vec2(1,0));

            Recipe* selectedRecipe = nullptr;

            for (auto& recipe : furnace.recipes)
            {

                if(itemSlot->draw(context,item,recipe->result)) {
                    selectedRecipe = recipe;
                }
                // context.drawRect(item,slots,solid);
                // context.drawRect(item,Color::white,recipe->result.item->getIcon());
                // if(context.mouseInside(item)) {
                //     selectedRecipe = recipe;
                // }

                // if(recipe->result.amount > 1) {
                //     std::string str = "" + std::to_string((int)recipe->result.amount);

                //     vec2 position = vec2(0.0f);
                //     std::reverse(str.begin(),str.end());
                //     for(char c : str) {
                //         context.drawRect(Rect::anchored(Rect::withPivot(position,vec2(width,width*ratio),vec2(1,1)),item,vec2(1,1)),Color::white,font->getSprite(c));
                //         position.x -= width + padding;
                        
                //     }
                // }
                
                item.position.x -= slotSize.x + spacing;
                
            }

            if(selectedRecipe != nullptr) {
                drawTooltip(context,*selectedRecipe);
                if(context.mouseLeftClicked()) {
                    furnace.tryStartCraft(*selectedRecipe,user,storage,state);
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