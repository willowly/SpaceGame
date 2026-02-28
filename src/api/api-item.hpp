#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "item/item.hpp"
#include "item/item-stack.hpp"

namespace API {

    void loadAPIItem(sol::state& lua) {
        
        sol::usertype<Item> item = lua.new_usertype<Item>("item",sol::no_constructor);
        
        
        item["defaultSprite"] = &Item::defaultSprite;
        item["name"] = &Item::name;


        sol::usertype<ItemStack> itemStack = lua.new_usertype<ItemStack>("item_stack",sol::constructors<ItemStack(Item*,int)>());

        itemStack["item"] = &ItemStack::item;
        itemStack["amount"] = &ItemStack::amount;
    }
}