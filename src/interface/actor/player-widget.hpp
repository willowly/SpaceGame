
#pragma once

#include "engine/debug.hpp"

#include "actor-widget.hpp"
#include "actor/character.hpp"
#include "interface/actor/inventory-widget.hpp"
#include "interface/actor/toolbar-widget.hpp"
#include "interface/tooltip-widget.hpp"
#include "interface/recipe-slot-widget.hpp"

//currently the root widget of everything at this point
class PlayerWidget : public ActorWidget<Character> {

    public:

        Sprite cursorRectSprite;
        InventoryWidget* inventoryWidget = {};
        vec2 inventorySize = {};
        vec2 characterPanelSize = {};
        ToolbarWidget* toolbarWidget = {};
        ItemSlotWidget* cursorSlotWidget = {};
        TextWidget* speedText = {};
        vec2 cursorSlotSize = vec2(60,60);
        TooltipWidget* tooltipWidget = {};
        
        float cursorRectLength = 15;
        float cursorRectWidth = 3;
        
        //character panel
        int padding = 3;
        int spacing = 2;
        RecipeSlotWidget* recipeSlot = {};


    virtual void draw(DrawContext context,Character& player) {
        if(inventoryWidget == nullptr) {
            Debug::warn("Inventory Widget is null!");
            return;
        }
        if(toolbarWidget == nullptr) {
            Debug::warn("Toolbar Widget is null!");
            return;
        }
        if(cursorSlotWidget == nullptr) {
            Debug::warn("Cursor Slot Widget is null!");
            return;
        }
        if(speedText == nullptr) {
            Debug::warn("Speed Text Widget is null!");
            return;
        }
        TooltipTarget target;
        context.setTooltip = [&](TooltipTarget newTarget) {
            target = newTarget;
        };
        if(player.inMenu) {
            if(player.openMenuObject != nullptr) {
                player.openMenuObject->drawMenu(context,player);
            } else {
                characterPanel(context,player);
            }
            Rect inventoryPanel = Rect::anchored(Rect::withPivot(vec2(-3,0),inventorySize,vec2(1,0.5f)),context.getScreenSize(),vec2(0.5,0.5));
            inventoryWidget->draw(context,inventoryPanel,player,player.inventory);

            
        }
        toolbarWidget->draw(context,player);
        
        if(player.inMenu && player.cursorStack.isEmpty()) {
            tooltipWidget->draw(context,target);
        }

        Rect cursorStackRect = Rect(context.getMousePosition(),cursorSlotSize);
        cursorSlotWidget->draw(context,cursorStackRect,player.cursorStack);

        
        Rect screen = context.getScreenSize();
        float speed = glm::length(player.getVelocity());
        speedText->draw(context,vec2(screen.size.x*0.5f,screen.size.y-200),std::format("{0:.2f}",speed));

        context.drawRect(Rect::anchored(Rect::centered(vec2(cursorRectLength,cursorRectWidth)),screen,vec2(0.5,0.5)),cursorRectSprite);
        context.drawRect(Rect::anchored(Rect::centered(vec2(cursorRectWidth,cursorRectLength)),screen,vec2(0.5,0.5)),cursorRectSprite);
    }


    void characterPanel(DrawContext context,Character& player) {
        if(recipeSlot == nullptr) {
            Debug::warn("recipe slot is null (character panel)");
            return;
        }
        Rect characterPanel = Rect::anchored(Rect::withPivot(vec2(3,0),characterPanelSize,vec2(0,0.5f)),context.getScreenSize(),vec2(0.5,0.5));
        context.drawRect(characterPanel,inventoryWidget->backgroundSprite,inventoryWidget->backgroundColor);

        vec2 recipePos = characterPanel.position + vec2(static_cast<int>(padding));

        for (auto recipe : player.recipes)
        {
            assert(recipe != nullptr);
            if(recipeSlot->draw(context,recipePos,*recipe)) {
                if(context.mouseLeftClicked()) {
                    player.startCraft(*recipe);
                }
            }
            recipePos.x += recipeSlot->size.x + static_cast<int>(spacing);
        }
        
    }

    string getTypeName() override {
        return "player_widget";
    }

    
};