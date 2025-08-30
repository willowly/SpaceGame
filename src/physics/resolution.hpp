#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <optional>
#include <iostream>
#include "engine/debug.hpp"
#include "helper/math-helper.hpp"
#include "physics/structs.hpp"

using glm::vec3,std::optional,MathHelper::sign;

namespace Physics {

    void resolveBasic(vec3& position,Contact contact) {
        position += contact.normal * contact.penetration;
    }

}