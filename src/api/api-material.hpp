#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "graphics/material.hpp"
#include "graphics/color.hpp"

using std::unique_ptr;

namespace API {


    void loadAPIMaterial(sol::state& lua) {

        sol::usertype<Color> color = lua.new_usertype<Color>("color",sol::call_constructor,sol::constructors<Color(float,float,float),Color(float,float,float,float)>());
        
        sol::usertype<Material> material = lua.new_usertype<Material>("material",sol::no_constructor);

        material["shader"] = &Material::shader;
        material["texture"] = &Material::texture;
        material["color"] = &Material::color;

    };

}