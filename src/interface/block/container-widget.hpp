#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"
#include "block/container-block.hpp"
#include "block-widget.hpp"
#include "interface/item-slot-interact-options.hpp"

class ContainerWidget : public BlockWidget<ContainerBlock> {
    
    public:
        Sprite solid;
        vec2 size = vec2(380,80);
        float padding = 3;
        float margin = 5;
        float spacing = 2;
        int columns = 8;

        ItemSlotWidget* itemSlot;

        TextWidget* tooltipTextTitle;

        void draw(DrawContext context,Character& user,ContainerBlock& container,BlockStorage& storage) {

            if(itemSlot == nullptr) {
                Debug::warn("item slot is null (container widget)");
            }

            Rect screen = context.getScreenSize();

            auto backgroundColor = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = Rect::anchored(Rect::withPivot(vec2(0,-padding),size,vec2(0.5,1)),screen,vec2(0.5,0.5));
            context.drawRect(mainPanel,solid,backgroundColor);

            auto slotPosition = mainPanel.position;
            slotPosition += vec2(margin);

            auto inventory = container.getInventory(storage);

            bool hoveringPanel = context.mouseInside(mainPanel);
            int column = 0;
            for (auto stackPtr : inventory.getItems())
            {
                if(stackPtr == nullptr) continue;

                if(itemSlot->draw(context,slotPosition,*stackPtr)) {
                    user.itemSlotHoverActions(context,*stackPtr);
                    hoveringPanel = false;
                }

                slotPosition.x += (itemSlot->size.x + spacing);
                column++;
                if(column == columns) {
                    slotPosition.x = mainPanel.topLeft().x + margin;
                    slotPosition.y += (itemSlot->size.y + spacing);
                    column = 0;
                }
            }

            if(hoveringPanel) {
                ItemStack stack;
                if(user.itemSlotHoverActions(context,stack)) {
                    inventory.give(stack);
                }
            }
            
            
        }

        void drawStack(DrawContext context,Character& user,Rect mainPanel,BlockStorage& storage,int i) {
            
        }

        void drawTooltip(DrawContext context,ItemStack& stack) {
            
            Rect tooltip = Rect(context.getMousePosition()+vec2(3,3),vec2(40,7));
            tooltip.size.x = tooltipTextTitle->getSize(stack.item->displayName).x + 4.0f;
            context.drawRect(tooltip,solid,Color(0.05,0.05,0.05));

            tooltipTextTitle->draw(context,tooltip.position + vec2(2.0f,2.0f),stack.item->displayName);
        }

        string getTypeName() override {
            return "container_widget";
        }
};