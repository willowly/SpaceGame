#pragma once

#include <include/glm/glm.hpp>
#include <optional>
#include <iostream>
#include "engine/debug.hpp"
#include "helper/math-helper.hpp"

using glm::vec3,std::optional,MathHelper::sign;

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

    vec3 boxNormalAt(vec3 position,vec3 halfSize,vec3 point) {
            vec3 normal;
            vec3 localPoint = point - position;
            float min = std::numeric_limits<float>::max();
            vec3 size = halfSize * 2.0f; // :shrug:
            float distance = std::abs(size.x - std::abs(localPoint.x));
            if (distance < min) {
                min = distance;
                normal = vec3(1,0,0);
                normal *= sign(localPoint.x);
            }
            distance = std::abs(size.y - std::abs(localPoint.y));
            if (distance < min) {
                min = distance;
                normal = vec3(0,1,0);
                normal *= sign(localPoint.y);
            }
            distance = std::abs(size.z - std::abs(localPoint.z));
            if (distance < min) { 
                min = distance; 
                normal = vec3(0,0,1);
                normal *= sign(localPoint.z);
            } 
            return normal;
        }

    
    std::optional<RaycastHit> intersectRayPlane(Ray plane,Ray ray) {

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

    // TODO: make intersect box hollow with dstToBox = dstB - dstToBox;

    std::optional<RaycastHit> intersectRayBox(vec3 position,vec3 halfSize,Ray ray) {
        vec3 min = position - halfSize;
        vec3 max = position + halfSize;

        vec3 t0 = (min - ray.origin) / ray.direction;
        vec3 t1 = (max - ray.origin) / ray.direction;
        vec3 tmin = glm::min(t0,t1);
        vec3 tmax = glm::max(t0,t1);

        float dstA = glm::max(glm::max(tmin.x,tmin.y),tmin.z);
        float dstB = glm::min(tmax.x,glm::min(tmax.y,tmax.z));

        //ray misses box
        if(dstA > dstB) {
            return std::nullopt;
        }
        
        float dstToBox = dstA;

        //ray starts inside
        if (dstA < 0 && 0 < dstB) {
            dstToBox = 0; //if the box is solid, it should just hit immediatly
        }


        if(dstToBox < 0) {
            return std::nullopt;
        }

        
        
        vec3 point = ray.origin + ray.direction * dstToBox;
        vec3 normal = boxNormalAt(position,halfSize,point);
        
        return RaycastHit(point,normal,dstToBox); 
    }


};