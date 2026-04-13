
#pragma once

#include "engine/debug.hpp"

#include "actor-widget.hpp"
#include "actor/character.hpp"
#include "interface/actor/inventory-widget.hpp"
#include "interface/actor/toolbar-widget.hpp"

//currently the root widget of everything at this point
class PlayerWidget : public ActorWidget<Character> {

    public:

        Sprite cursorRectSprite;
        InventoryWidget* inventoryWidget;
        ToolbarWidget* toolbarWidget;
        ItemSlotWidget* cursorSlotWidget;
        vec2 cursorSlotSize = vec2(60,60);

        float cursorRectLength = 15;
        float cursorRectWidth = 3;


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

        Rect cursorStackRect = Rect(context.getMousePosition(),cursorSlotSize);
        cursorSlotWidget->draw(context,cursorStackRect,player.cursorStack);

        Rect screen = context.getScreenSize();

        context.drawRect(Rect::anchored(Rect::centered(vec2(cursorRectLength,cursorRectWidth)),screen,vec2(0.5,0.5)),cursorRectSprite);
        context.drawRect(Rect::anchored(Rect::centered(vec2(cursorRectWidth,cursorRectLength)),screen,vec2(0.5,0.5)),cursorRectSprite);
    }

    
};