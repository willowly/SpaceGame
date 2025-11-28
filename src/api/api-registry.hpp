#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include <api/api-registry-general.hpp>
#include <api/api-registry-material.hpp>
#include <api/api-registry-actor.hpp>
#include <api/api-registry-block.hpp>
#include <api/api-registry-item.hpp>

using std::string,std::variant;

namespace API {

    void loadAPIRegistry(sol::state& lua) {

        sol::usertype<TextureRegistry> textureRegistry = lua.new_usertype<TextureRegistry>("textureRegistry",sol::no_constructor);

        textureRegistry["__index"] = &TextureRegistry::index;
        textureRegistry["__newindex"] = &TextureRegistry::newindex;

        // sol::usertype<ShaderRegistry> shaderRegistry = lua.new_usertype<ShaderRegistry>("shaderRegistry",sol::no_constructor);

        // shaderRegistry["lit"] = sol::property(&ShaderRegistry::litShader);

        sol::usertype<MaterialRegistry> materialRegistry = lua.new_usertype<MaterialRegistry>("materialRegistry",sol::no_constructor);

        materialRegistry["__index"] = &MaterialRegistry::index;
        materialRegistry["__newindex"] = &MaterialRegistry::newindex;

        sol::usertype<ActorRegistry> actorRegistry = lua.new_usertype<ActorRegistry>("actorRegistry",sol::no_constructor);

        actorRegistry["__index"] = &ActorRegistry::index;
        actorRegistry["__newindex"] = &ActorRegistry::newindex;


        sol::usertype<BlockRegistry> blockRegistry = lua.new_usertype<BlockRegistry>("blockRegistry",sol::no_constructor);

        blockRegistry["__index"] = &BlockRegistry::index;
        blockRegistry["__newindex"] = &BlockRegistry::newindex;

        sol::usertype<ItemRegistry> itemRegistry = lua.new_usertype<ItemRegistry>("itemRegistry",sol::no_constructor);

        itemRegistry["__index"] = &ItemRegistry::index;
        itemRegistry["__newindex"] = &ItemRegistry::newindex;

    }

}