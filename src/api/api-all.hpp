#pragma once

#include <sol/sol.hpp>

#include "api-general.hpp"
#include "api-texture.hpp"
#include "api-material.hpp"
#include "api-actor.hpp"
#include "api-character.hpp"
#include "api-construction.hpp"
#include "api-world.hpp"
#include "api-registry.hpp"
#include "api-block.hpp"
#include "api-item.hpp"

namespace API {

    void loadAPIAll(sol::state& lua) {
        loadAPIGeneral(lua);
        loadAPITexture(lua);
        loadAPIMaterial(lua);
        loadAPIActor(lua);
        loadAPICharacter(lua);
        loadAPIConstruction(lua);
        loadAPIWorld(lua);
        loadAPIBlock(lua);
        loadAPIItem(lua);
        loadAPIRegistry(lua);
    }
}