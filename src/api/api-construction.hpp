#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include <actor/construction.hpp>

namespace API {

    void loadAPIConstruction(sol::state& lua) {

        sol::usertype<ivec3> vector3Int = lua.new_usertype<ivec3>("ivec3",sol::constructors<glm::ivec3(int,int,int)>());

        sol::usertype<Construction> construction = lua.new_usertype<Construction>("construction",sol::no_constructor);

        construction["setBounds"] = &Construction::setBounds;
        construction["setBlock"] = &Construction::setBlock;
    }
}