#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "actor/character.hpp"

namespace API {

    void loadAPICharacter(sol::state& lua) {
        
        sol::usertype<Character> character = lua.new_usertype<Character>("character",sol::no_constructor);
        
        character["give"] = sol::overload(
            [&](Character* character,Item* item,int amount) {
                character->inventory.give(ItemStack(item,amount));
            },
            [&](Character* character,ItemStack stack) {
                character->inventory.give(stack);
            }
        );

        character["setToolbar"] = [&](Character* character,ItemStack stack,int index) {
            if(index < 0 || index > 9) {
                throw sol::error("toolbar index out of range");
            }
            character->toolbar[index] = stack;
        };

    }
}