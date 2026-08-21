#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"

class InventoryWidget : public Widget {
    
    public:
        Sprite backgroundSprite;
        float margin = 5;
        float spacing = 2;
        int columns = 8;

        ItemSlotWidget* itemSlot;

        TextWidget* tooltipTextTitle;

        void draw(DrawContext context,Rect rect,Character& user,IInventory& inventory) {

            if(itemSlot == nullptr) {
                Debug::warn("item slot is null (inventory widget)");
            }

            Rect screen = context.getScreenSize();

            auto backgroundColor = Color(0.2,0.2,0.2);
            auto slots = Color(0.1,0.1,0.1);
            auto slotsHover = Color(0.1,0.1,1);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            
            Rect mainPanel = rect;
            context.drawRect(mainPanel,backgroundSprite,backgroundColor);
            
            ItemSlotInteractOptions interactOptions;
            
            if(inventory.maxWeight == 0) {
                tooltipTextTitle->draw(context,mainPanel.topLeft() + vec2(margin),std::format("{:0}",inventory.getTotalWeight()));
            } else {
                tooltipTextTitle->draw(context,mainPanel.topLeft() + vec2(margin),std::format("{:0}/{:0}",inventory.getTotalWeight(),inventory.maxWeight));
                interactOptions.spaceLeft = inventory.getSpaceLeft();
            }

            auto slotPosition = mainPanel.position;
            slotPosition += vec2(margin);
            slotPosition.y += tooltipTextTitle->height;
            slotPosition.y += margin;

            

            bool hoveringPanel = context.mouseInside(mainPanel);
            int column = 0;
            for (auto stackPtr : inventory.getItems())
            {
                if(stackPtr == nullptr) continue;

                if(itemSlot->draw(context,slotPosition,*stackPtr)) {
                    if(user.cursorStack.isEmpty()) {
                        user.itemSlotHoverActions(context,*stackPtr,interactOptions);
                        hoveringPanel = false;
                    }
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
                ItemStack stack = user.cursorStack;
                user.inventoryHoverActions(context,inventory);
            }
            
            
        }

        string getTypeName() {
            return "inventory_widget";
        }
};