#pragma once
#include "physics/intersections.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class ToolUser {

    public:
        virtual Ray getLookRay() = 0;

        virtual quat getEyeRotation() = 0;

        virtual glm::mat4 getTransform() = 0;

        virtual glm::vec3 getEyePosition() = 0;
};