#pragma once

#include <sol/sol.hpp>
#include "glm/glm.hpp"

using std::variant, std::string,glm::vec3;

namespace API {

    void loadAPIGeneral(sol::state& lua) {
        // to not be the same as the type name
        lua["vec3"] = [&](float x,float y,float z) {
            return vec3(x,y,z);
        };

        lua["quat"] = [&](float x,float y,float z) {
            return quat(glm::vec3(glm::radians(x),glm::radians(y),glm::radians(z)));
        };
    }

}