#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <Jolt/Jolt.h>


namespace Physics {

    JPH::Vec3 toJoltVec(glm::vec3 v) {
        return JPH::Vec3(v.x,v.y,v.z);
    }

    glm::vec3 toGlmVec(JPH::Vec3 v) {
        return glm::vec3(v.GetX(),v.GetY(),v.GetZ());
    }

    JPH::Quat toJoltQuat(glm::quat q) {
        return JPH::Quat(q.x,q.y,q.z,q.w);
    }

    glm::quat toGlmQuat(JPH::Quat q) {
        return glm::quat(q.GetW(),q.GetX(),q.GetY(),q.GetZ());
    }
}