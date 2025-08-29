#pragma once
#include "collision/intersections.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class ToolUser {

    public:
        virtual Ray getLookRay() = 0;

        virtual quat getEyeRotation() = 0;
};