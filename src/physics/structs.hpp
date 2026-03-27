#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <optional>
#include <iostream>
#include "engine/debug.hpp"
#include "helper/math-helper.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/ContactListener.h>

class Actor;

using glm::vec3,std::optional,MathHelper::sign;

struct RaycastHit {
    vec3 point = {};
    vec3 normal = {};
    float distance;
    RaycastHit(vec3 point,vec3 normal,float distance) : point(point),normal(normal),distance(distance) { }
};

struct Ray {
    vec3 origin = {};
    vec3 direction = {};
    Ray(vec3 origin,vec3 direction) : origin(origin), direction(direction) { }
};

struct Collision {
    Actor* actor;
    JPH::BodyID body;
    Actor* otherActor;
    JPH::BodyID otherBody;
    const JPH::ContactManifold &inManifold;
};