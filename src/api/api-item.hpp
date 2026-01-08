#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "item/item.hpp"

namespace API {

    void loadAPIItem(sol::state& lua) {
        
        sol::usertype<Item> item = lua.new_usertype<Item>("item",sol::no_constructor);
        
        
        item["defaultSprite"] = &Item::defaultSprite;
        item["name"] = &Item::name;
    }
}