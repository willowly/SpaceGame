#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"
#include <interface/widgets-all.hpp>

using std::string,std::variant;

namespace API {
    
    void addTextWidget(string name,sol::table table,Registry& registry) {
        TextWidget* widget = registry.addWidget<TextWidget>(name);
        get<float>(table,"height",widget->height);
        get<float>(table,"ratio",widget->ratio);
        get<float>(table,"spacing",widget->spacing);
        get<Color>(table,"color",widget->color);
        widget->font = &registry.font;
        Debug::info("Loaded Text Widget \"" + name + "\"",InfoPriority::MEDIUM);
    }

    void addItemSlotWidget(string name,sol::table table,Registry& registry) {
        ItemSlotWidget* widget = registry.addWidget<ItemSlotWidget>(name);
        getSprite(table,"sprite",widget->sprite,registry);
        get<Color>(table,"color",widget->color);
        get<float>(table,"bar_width",widget->barWidth);
        get<float>(table,"padding",widget->padding);
        widget->font = &registry.font;
        Debug::info("Loaded Item Slot Widget \"" + name + "\"",InfoPriority::MEDIUM);
    }

    // void addInventoryWidget(string name,sol::table table,Registry& registry) {
    //     InventoryWidget* widget = registry.addWidget<InventoryWidget>(name);
    //     getSprite(table,"background_sprite",widget->backgroundSprite,registry);
    //     get<vec2>(table,"size",widget->size);
    //     get<float>(table,"padding",widget->padding);
    //     get<vec2>(table,"slot_size",widget->slotSize);
    //     get<float>(table,"spacing",widget->spacing);
    //     getWidget<ItemSlotWidget>(table,"item_slot",widget->itemSlot,registry,true);
    //     getWidget<TextWidget>(table,"tooltip_text_title",widget->tooltipTextTitle,registry,true);
    //     widget->font = &registry.font;
    //     Debug::info("Loaded Inventory Widget \"" + name + "\"",InfoPriority::MEDIUM);
    // }

    void addWidgetWithTypeAndLoad(string type,string name,ObjLoadType loadType,sol::table table,Registry& registry) {
        if(loadType == ObjLoadType::INVALID) {
            Debug::warn("trying to load with invalid object");
            return;
        }
        if(loadType == ObjLoadType::ARRAY) {
            Debug::warn("loading widgets in array mode is not supported");
        }

        if(type == "text") {
            addTextWidget(name,table,registry);
            return;
        }
        if(type == "item_slot") {
            addItemSlotWidget(name,table,registry);
            return;
        }
        if(type == "inventory") {
            //addInventoryWidget(name,table,registry);
            return;
        }
        Item* block = registry.addItem<ResourceItem>(name);
        loadItemBaseType(loadType,table,block,registry);
        Debug::info("Loaded Item \"" + name + "\"",InfoPriority::MEDIUM);
        
    }


    struct WidgetRegistry {
        WidgetRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Item* index(string name) {
            return registry.getItem(name);
        }

        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("widget");
            Debug::addTrace(name);

            sol::table table = obj; //this will be null if it doesnt work, so be careful

            string type = "";
            ObjLoadType loadType = getObjectLoadType(obj);
            switch (getObjectLoadType(obj)) {
                case ObjLoadType::ARRAY:
                    get<string>(table,1,type,true);
                    break;
                case ObjLoadType::TABLE:
                    get<string>(table,"type",type,true);
                    break;
                case ObjLoadType::INVALID:
                    type = "invalid";
                    Debug::warn("object is invalid");
                    break;
            }

            if(type != "invalid") {
                addWidgetWithTypeAndLoad(type,name,loadType,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}