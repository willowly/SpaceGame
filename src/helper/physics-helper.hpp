#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

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

    struct RaycastCallback : public rp3d::RaycastCallback {

        public:

            bool success = false;
            glm::vec3 worldPoint;
            glm::vec3 worldNormal;
            rp3d::Body* body;


            rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) {

                
                success= true;

                worldPoint = toGlmVector(info.worldPoint);
                worldNormal = toGlmVector(info.worldNormal);
                body = info.body;

                
                return info.hitFraction;
            }
    };
}

namespace ph = PhysicsHelper;