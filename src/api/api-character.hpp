#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "actor/character.hpp"

namespace API {

    void loadAPICharacter(sol::state& lua) {
        
        sol::usertype<Character> character = lua.new_usertype<Character>("character",sol::no_constructor);
        
        character["give"] = [&](Character* character,Item* item,int amount) {
            character->inventory.give(item,amount);
        };
        character["setToolbar"] = [&](Character* character,Item* item,int index) {
            if(index < 0 || index > 9) {
                throw sol::error("toolbar index out of range");
            }
            character->toolbar[index] = item;
        };

    }
}