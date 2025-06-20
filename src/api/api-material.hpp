#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "graphics/material.hpp"

using std::unique_ptr;

namespace API {


    void loadAPIMaterial(sol::state& lua) {
        
        sol::usertype<Material> material = lua.new_usertype<Material>("material",sol::no_constructor);

        material["shader"] = &Material::shader;
        material["texture"] = &Material::texture;

    };

}