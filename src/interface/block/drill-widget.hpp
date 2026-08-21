#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"
#include "interface/text-widget.hpp"
#include "interface/item-slot-widget.hpp"
#include "block/drill-block.hpp"
#include "block-widget.hpp"
#include "interface/item-slot-interact-options.hpp"

class DrillWidget : public BlockWidget<DrillBlock> {
    
    public:
        vec2 size = vec2(380,80);
        int padding = 3;

        InventoryWidget* inventory;


        void draw(DrawContext context,Character& user,DrillBlock& drill,BlockStorage& storage) {

            if(inventory == nullptr) {
                Debug::warn("item slot is null (container widget)");
            }

            Rect rect = Rect::anchored(Rect::withPivot(vec2(padding,0),size,vec2(0,0.5)),context.getScreenSize(),vec2(0.5,0.5));

            auto blockInventory = drill.getInventory(storage);

            inventory->draw(context,rect,user,blockInventory);
            
            
        }
        
        string getTypeName() override {
            return "drill_widget";
        }
};