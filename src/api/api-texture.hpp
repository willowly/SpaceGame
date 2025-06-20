#pragma once

#include <sol/sol.hpp>

#include "api-general.hpp"
#include "graphics/texture.hpp"

namespace API {

    void loadAPITexture(sol::state& lua) {

        sol::usertype<Texture> texture = lua.new_usertype<Texture>("texture",sol::no_constructor);

        texture["setLinearFiltering"] = &Texture::setLinearFiltering;

        texture["setPointFiltering"] = &Texture::setPointFiltering;

    }
}