#pragma once

#include <sol/sol.hpp>
#include "engine/world.hpp"

namespace API {

    inline void loadAPIWorld(sol::state& lua) {
        sol::usertype<World> world = lua.new_usertype<World>("world",sol::no_constructor);


        world["spawnDummy"] = [&] (World& world,Actor* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            world.spawn(Actor::makeInstance(prototype,position,rotation));
        };

        world["spawnPlayer"] = [&] (World& world,Character* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            world.spawn(Character::makeInstance(prototype,position,rotation));
        };

        // world["spawnTerrain"] = [&] (World& world,Character* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
        //     world.spawn(Character::makeInstance(prototype,position,rotation));
        // };
    }
}