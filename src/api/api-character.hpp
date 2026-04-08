#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "actor/character.hpp"

namespace API {

    void loadAPICharacter(sol::state& lua) {
        
        sol::usertype<Character> character = lua.new_usertype<Character>("character",sol::no_constructor);

        character["moveSpeed"] = &Character::moveSpeed;
        
        character["give"] = sol::overload(
            [&](Character* character,Item* item,int amount) {
                if(character == nullptr) {
                    throw sol::error("character is null (give)");
                }
                character->give(ItemStack(item,amount));
            },
            [&](Character* character,ItemStack stack) {
                if(character == nullptr) {
                    throw sol::error("character is null (give)");
                }
                character->give(stack);
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