#pragma once
#include "type-info.hpp"
#include "interface/widgets-all.hpp"

#define WIDGET_TYPE(widgetClass,name) \
    TypeInfo* name = registry.addTypeInfo<widgetClass>(#name); \
    name->setParent(widget);

namespace TypeInfoLoader {

    inline void loadWidget(Registry& registry) {

        auto widget = registry.addTypeInfo<Widget>("widget");
        
        TypeInfo* inventoryWidget = registry.addTypeInfo<InventoryWidget>("inventory_widget");
        //inventoryWidget->parent = widget;
        inventoryWidget->constructorFunction = [&]() { return std::make_unique<InventoryWidget>(); };
        inventoryWidget->addConstProperty("background_sprite",&InventoryWidget::backgroundSprite);
        inventoryWidget->addConstProperty("font",&InventoryWidget::font);
        inventoryWidget->addConstProperty("size",&InventoryWidget::size);
        inventoryWidget->addConstProperty("padding",&InventoryWidget::padding);
        inventoryWidget->addConstProperty("slot_size",&InventoryWidget::slotSize);
        inventoryWidget->addConstProperty("spacing",&InventoryWidget::spacing);
        inventoryWidget->addConstProperty("item_slot",&InventoryWidget::itemSlot);
        inventoryWidget->addConstProperty("tooltip_text_title",&InventoryWidget::tooltipTextTitle);
        inventoryWidget->addConstProperty("recipe_slot",&InventoryWidget::recipeSlot);
        inventoryWidget->setParent(widget);

        TypeInfo* toolbarWidget = registry.addTypeInfo<ToolbarWidget>("toolbar_widget");
        toolbarWidget->constructorFunction = [&]() { return std::make_unique<ToolbarWidget>(); };
        //toolbarWidget->parent = widget;
        toolbarWidget->addConstProperty("item_slot_sprite",&ToolbarWidget::itemSlotSprite);
        toolbarWidget->addConstProperty("sprite",&ToolbarWidget::sprite);
        toolbarWidget->addConstProperty("selector_sprite",&ToolbarWidget::selectorSprite);
        toolbarWidget->addConstProperty("item_slot",&ToolbarWidget::itemSlot);
        toolbarWidget->addConstProperty("slot_size",&ToolbarWidget::slotSize);
        toolbarWidget->addConstProperty("slot_gap",&ToolbarWidget::slotGap);
        toolbarWidget->addConstProperty("slot_height",&ToolbarWidget::slotHeight);
        toolbarWidget->addConstProperty("selector_size",&ToolbarWidget::selectorSize);
        toolbarWidget->setParent(widget);

        TypeInfo* playerWidget = registry.addTypeInfo<PlayerWidget>("player_widget");
        //playerWidget->parent = widget;
        playerWidget->constructorFunction = [&]() { return std::make_unique<PlayerWidget>(); };
        playerWidget->addConstProperty("cursor_rect_sprite",&PlayerWidget::cursorRectSprite);
        playerWidget->addConstProperty("inventory_widget",&PlayerWidget::inventoryWidget);
        playerWidget->addConstProperty("toolbar_widget",&PlayerWidget::toolbarWidget);
        playerWidget->addConstProperty("cursor_slot_widget",&PlayerWidget::cursorSlotWidget);
        playerWidget->addConstProperty("speed_text",&PlayerWidget::speedText);
        playerWidget->addConstProperty("cursor_slot_size",&PlayerWidget::cursorSlotSize);
        playerWidget->addConstProperty("cursor_rect_length",&PlayerWidget::cursorRectLength);
        playerWidget->addConstProperty("cursor_rect_width",&PlayerWidget::cursorRectWidth);
        playerWidget->setParent(widget);

        TypeInfo* furnaceWidget = registry.addTypeInfo<FurnaceWidget>("furnace_widget");
        //furnaceWidget->parent = widget;
        furnaceWidget->constructorFunction = [&]() { return std::make_unique<FurnaceWidget>(); };
        furnaceWidget->addConstProperty("solid",&FurnaceWidget::solid);
        furnaceWidget->addConstProperty("font",&FurnaceWidget::font);
        furnaceWidget->addConstProperty("size",&FurnaceWidget::size);
        furnaceWidget->addConstProperty("padding",&FurnaceWidget::padding);
        furnaceWidget->addConstProperty("slot_size",&FurnaceWidget::slotSize);
        furnaceWidget->addConstProperty("spacing",&FurnaceWidget::spacing);
        furnaceWidget->addConstProperty("item_slot",&FurnaceWidget::itemSlot);
        furnaceWidget->addConstProperty("recipe_slot",&FurnaceWidget::recipeSlot);
        furnaceWidget->addConstProperty("tooltip_text_title",&FurnaceWidget::tooltipTextTitle);
        furnaceWidget->setParent(widget);

        TypeInfo* itemSlotWidget = registry.addTypeInfo<ItemSlotWidget>("item_slot_widget");
        //itemSlotWidget->parent = widget;
        itemSlotWidget->constructorFunction = [&]() { return std::make_unique<ItemSlotWidget>(); };
        itemSlotWidget->addConstProperty("sprite",&ItemSlotWidget::sprite);
        itemSlotWidget->addConstProperty("bar_sprite",&ItemSlotWidget::barSprite);
        itemSlotWidget->addConstProperty("color",&ItemSlotWidget::color);
        itemSlotWidget->addConstProperty("font",&ItemSlotWidget::font);
        itemSlotWidget->addConstProperty("text",&ItemSlotWidget::text);
        itemSlotWidget->addConstProperty("bar_width",&ItemSlotWidget::barWidth);
        itemSlotWidget->addConstProperty("padding",&ItemSlotWidget::padding);
        itemSlotWidget->setParent(widget);


        TypeInfo* textWidget = registry.addTypeInfo<TextWidget>("text_widget");
        //textWidget->parent = widget;
        textWidget->constructorFunction = [&]() { return std::make_unique<TextWidget>(); };
        textWidget->addConstProperty("height",&TextWidget::height);
        textWidget->addConstProperty("ratio",&TextWidget::ratio);
        textWidget->addConstProperty("spacing",&TextWidget::spacing);
        textWidget->addConstProperty("color",&TextWidget::color);
        textWidget->addConstProperty("font",&TextWidget::font);
        textWidget->setParent(widget);

        TypeInfo* containerWidget = registry.addTypeInfo<ContainerWidget>("container_widget");
        //furnaceWidget->parent = widget;
        containerWidget->constructorFunction = [&]() { return std::make_unique<ContainerWidget>(); };
        containerWidget->addConstProperty("solid",&ContainerWidget::solid);
        containerWidget->addConstProperty("size",&ContainerWidget::size);
        containerWidget->addConstProperty("padding",&ContainerWidget::padding);
        containerWidget->addConstProperty("spacing",&ContainerWidget::spacing);
        containerWidget->addConstProperty("item_slot",&ContainerWidget::itemSlot);
        containerWidget->addConstProperty("tooltip_text_title",&ContainerWidget::tooltipTextTitle);
        containerWidget->setParent(widget);


        TypeInfo* font = registry.addTypeInfo<Font>("font");

        WIDGET_TYPE(BlockWidget<FurnaceBlock>, blockWidget_Furnace);
        WIDGET_TYPE(ActorWidget<Character>, actorWidget_Character);
        WIDGET_TYPE(BlockWidget<ContainerBlock>, blockWidget_Container);


    }

}