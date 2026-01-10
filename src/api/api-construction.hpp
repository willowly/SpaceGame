#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include <actor/construction.hpp>
#include <helper/math-helper.hpp> //for ivec3 idk lol

bool compareIVec3(const ivec3& a, const ivec3& b)
{
    if(a.x == b.x) {
        if(a.y == b.y) {
            return a.z < b.z;
        }
        return a.y < b.y;
    }
    return a.x < b.x;
}

namespace API {

    void loadAPIConstruction(sol::state& lua) {

        sol::automagic_enrollments enrollments;
        enrollments.less_than_operator = false;
        enrollments.less_than_or_equal_to_operator = false;

        // sol::usertype<ivec3> vector3Int = lua.new_usertype<ivec3>("ivec3",
        //                                                         sol::call_constructor,sol::constructors<glm::ivec3(int,int,int)>(),
        //                                                         sol::meta_function::less_than, &compareIVec3
                                                            
        //                                                     );

        sol::usertype<vec3> vector3 = lua.new_usertype<vec3>("vec3",sol::call_constructor,sol::constructors<glm::vec3(float,float,float)>());

        sol::usertype<Construction> construction = lua.new_usertype<Construction>("construction",sol::no_constructor);

        construction["setBounds"] = &Construction::setBounds;
        construction["placeBlock"] = &Construction::placeBlock;
        construction["breakBlock"] = &Construction::breakBlock;
        construction["position"] = sol::property(&Construction::getPosition,&Construction::setPosition);
        construction["rotate"] = &Construction::rotate;
    }
}