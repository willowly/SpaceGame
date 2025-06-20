#pragma once

#include <include/glm/glm.hpp>
#include <optional>
#include <iostream>

using glm::vec3,std::optional;

namespace CollisionHelper {

    struct RaycastHit {
        vec3 point;
        vec3 normal;
        float distance;
        RaycastHit(vec3 point,vec3 normal,float distance) : point(point),normal(normal),distance(distance) { }
    };

    struct Ray {
        vec3 origin;
        vec3 direction;
        Ray(vec3 origin,vec3 direction) : origin(origin), direction(direction) { }
    };

    
    std::optional<RaycastHit> intersectPlane(Ray plane,Ray ray) {

        // assuming vectors are normalized
        float denom = glm::dot(plane.direction, ray.direction);
        auto workingPlaneDirection = plane.direction;
        if(denom > 0) { //make sure the normal is pointing away from the ray?
            workingPlaneDirection *= -1.0f;
        }

        vec3 delta = plane.origin - ray.origin;
        float distance = glm::dot(delta,plane.direction) / denom;
        if(distance >= 0) {
            return RaycastHit(ray.origin + ray.direction * distance,plane.direction,distance);
        } else {
            std::cout << "distance less than 0" << std::endl;
            return std::nullopt;
        }

    }

};