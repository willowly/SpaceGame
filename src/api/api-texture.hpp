#pragma once

#include <sol/sol.hpp>

#include "api-general.hpp"

namespace API {

    void loadAPITexture(sol::state& lua) {

        //sol::usertype<Texture> texture = lua.new_usertype<TextureID>("texture",sol::no_constructor);

        // texture["setLinearFiltering"] = &Texture::setLinearFiltering;

        // texture["setPointFiltering"] = &Texture::setPointFiltering;

    }
}