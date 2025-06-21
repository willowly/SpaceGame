#pragma once
#include "actor.hpp"
#include <include/glm/glm.hpp>
#include <include/glm/gtc/quaternion.hpp>
#include <include/glm/gtx/quaternion.hpp>
#include <format>
#include <string>
#include <iostream>
#include "helper/math-helper.hpp"
#include "helper/string-helper.hpp"

#include "helper/collision-helper.hpp"

using CollisionHelper::RaycastHit,CollisionHelper::intersectRayBox,std::optional,glm::vec3,CollisionHelper::Ray;

class PhysicsActor : public Actor {
    public: 

        vec3 angularVelocity = vec3(0);

        PhysicsActor() {

        }
        PhysicsActor(Model* model,Material* material) : Actor(model,material) {

        }

        void step(float dt) {
            rotation = glm::quat(angularVelocity * dt) * rotation;

            Actor::step(dt);
        }

        void applyForce(vec3 force,vec3 point) {
            vec3 delta = point - position;
            vec3 torque = glm::cross(delta,force);
            applyTorque(torque);
            applyForce(force);
        }

        void applyForce(vec3 force) {
            velocity += force;
        }

        void applyTorque(vec3 torque) {
            angularVelocity += torque;
        }

        optional<RaycastHit> raycast(Ray ray) {
            return intersectRayBox(position,vec3(1.0f),rotation,ray);
        }
        

};