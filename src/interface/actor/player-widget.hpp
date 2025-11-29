
#pragma once

#include "engine/debug.hpp"

#include "actor-widget.hpp"
#include "actor/character.hpp"
#include "interface/actor/inventory-widget.hpp"
#include "interface/actor/toolbar-widget.hpp"

//currently the root widget of everything at this point
class PlayerWidget : public ActorWidget<Character> {

    public:

        InventoryWidget* inventoryWidget;
        ToolbarWidget* toolbarWidget;


    virtual void draw(DrawContext context,Character& player) {
        if(inventoryWidget == nullptr) {
            Debug::warn("Inventory Widget is null!");
            return;
        }
        if(toolbarWidget == nullptr) {
            Debug::warn("Toolbar Widget is null!");
            return;
        }
        if(player.inMenu) {
            if(player.openMenuObject != nullptr) {
                player.openMenuObject->drawMenu(context,player);
            }
            inventoryWidget->draw(context,player);
        }
        toolbarWidget->draw(context,player);
    }

    
};