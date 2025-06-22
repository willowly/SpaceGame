#pragma once

#include <include/glm/glm.hpp>
#include <include/glm/gtc/quaternion.hpp>
#include <include/glm/gtx/quaternion.hpp>

#include <reactphysics3d/reactphysics3d.h>


namespace PhysicsHelper {

    rp3d::Vector3 toRp3dVector(glm::vec3 v) {
        return rp3d::Vector3(v.x,v.y,v.z);
    }

    glm::vec3 toGlmVector(rp3d::Vector3 v) {
        return glm::vec3(v.x,v.y,v.z);
    }

    rp3d::Quaternion toRp3dQuaternion(glm::quat q) {
        return rp3d::Quaternion(q.x,q.y,q.z,q.w);
    }

    glm::quat toGlmQuaternion(rp3d::Quaternion q) {
        return glm::quat(q.w,q.x,q.y,q.z);
    }
}