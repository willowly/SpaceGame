#pragma once

#include <sol/sol.hpp>

#include "api-general.hpp"
#include "api-texture.hpp"
#include "api-material.hpp"
#include "api-actor.hpp"
#include "api-character.hpp"
#include "api-world.hpp"
#include "api-registry.hpp"

namespace API {

    void loadAPIAll(sol::state& lua) {
        loadAPITexture(lua);
        loadAPIMaterial(lua);
        loadAPIActor(lua);
        loadAPICharacter(lua);
        loadAPIWorld(lua);
        loadAPIRegistry(lua);
    }
}