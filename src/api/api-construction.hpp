#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include <actor/construction.hpp>

namespace API {

    void loadAPIConstruction(sol::state& lua) {

        sol::usertype<ivec3> vector3Int = lua.new_usertype<ivec3>("ivec3",sol::call_constructor,sol::constructors<glm::ivec3(int,int,int)>());

        sol::usertype<vec3> vector3 = lua.new_usertype<vec3>("vec3",sol::call_constructor,sol::constructors<glm::vec3(float,float,float)>());

        sol::usertype<Construction> construction = lua.new_usertype<Construction>("construction",sol::no_constructor);

        construction["setBounds"] = &Construction::setBounds;
        construction["setBlock"] = &Construction::setBlock;
        construction["position"] = &Construction::position;
        construction["rotate"] = &Construction::rotate;
    }
}