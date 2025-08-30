#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/closest_point.hpp>

#include <optional>
#include <iostream>
#include "engine/debug.hpp"
#include "helper/math-helper.hpp"
#include "physics/structs.hpp"

using glm::vec3,std::optional,MathHelper::sign;

namespace Physics {

    vec3 boxNormalAt(vec3 position,vec3 halfSize,vec3 point) {
            vec3 normal;
            vec3 localPoint = point - position;
            float min = std::numeric_limits<float>::max();
            //vec3 size = halfSize * 2.0f; // :shrug:
            float distance = std::abs(halfSize.x - std::abs(localPoint.x));
            if (distance < min) {
                min = distance;
                normal = vec3(1,0,0);
                normal *= sign(localPoint.x);
            }
            distance = std::abs(halfSize.y - std::abs(localPoint.y));
            if (distance < min) {
                min = distance;
                normal = vec3(0,1,0);
                normal *= sign(localPoint.y);
            }
            distance = std::abs(halfSize.z - std::abs(localPoint.z));
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
            return std::nullopt;
        }

    }

    std::optional<RaycastHit> intersectRayTriangle(vec3 a,vec3 b,vec3 c,Ray ray) {

        vec3 normal = MathHelper::normalFromPlanePoints(a,b,c);

        std::optional<RaycastHit> hit_opt = intersectRayPlane(Ray(a,normal),ray);

        if(!hit_opt) return hit_opt; //if theres no hit, dont bother with the other checks

        auto hit = hit_opt.value();

        //test if its on the correct side of each line
        if(glm::dot(hit.normal,glm::cross(b-a,hit.point-a)) < 0) {
            return std::nullopt;
        }
        if(glm::dot(hit.normal,glm::cross(c-b,hit.point-b)) < 0) {
            return std::nullopt;
        }
        if(glm::dot(hit.normal,glm::cross(a-c,hit.point-c)) < 0) {
            return std::nullopt;
        }

        return hit_opt;
    }

    // TODO: make intersect box hollow with dstToBox = dstB - dstToBox;

    std::optional<RaycastHit> intersectRayBox(vec3 halfSize,Ray ray) {
        vec3 min = -halfSize;
        vec3 max = halfSize;

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
        vec3 normal = boxNormalAt(vec3(0),halfSize,point);
        
        return RaycastHit(point,normal,dstToBox); 
    }

    std::optional<RaycastHit> intersectRayBox(vec3 position,vec3 halfSize,Ray ray) {

        ray.origin -= position;

        auto hitOpt = intersectRayBox(halfSize,ray);
        if(hitOpt) {
            auto hit = hitOpt.value();
            hit.point += position;
            return hit;
        }
        return std::nullopt;

    }

    std::optional<RaycastHit> intersectRayBox(vec3 position,vec3 halfSize,quat rotation,Ray ray) {


        ray.origin -= position;
        ray.origin = glm::inverse(rotation) * ray.origin;
        ray.direction = glm::inverse(rotation) * ray.direction;

        auto hitOpt = intersectRayBox(halfSize,ray);
        if(hitOpt) {
            auto hit = hitOpt.value();
            hit.point = rotation * hit.point;
            hit.normal = rotation * hit.normal;
            hit.point += position;
            return hit;
        }
        return std::nullopt;

    }

    std::optional<Contact> intersectSphereBox(vec3 spherePos,float radius,vec3 boxPos,vec3 halfSize) {
        vec3 delta = spherePos - boxPos;
        //delta = glm::inverse(boxOrientation) * delta;

        if (abs(delta.x) - radius > halfSize.x ||
            abs(delta.y) - radius > halfSize.y ||
            abs(delta.z) - radius > halfSize.z)
        {
            return std::nullopt; //early out
        }

        vec3 closestPoint;
        closestPoint.x = glm::clamp(delta.x,-halfSize.x,halfSize.x);
        closestPoint.y = glm::clamp(delta.y,-halfSize.y,halfSize.y);
        closestPoint.z = glm::clamp(delta.z,-halfSize.z,halfSize.z);

        vec3 deltaToPoint = delta - closestPoint;
        float deltaToPointLength = glm::length(deltaToPoint);
        float penetration = radius - deltaToPointLength; //TODO optimize to use the square
        if(deltaToPointLength == 0) {
            
            if(abs(delta.x) < abs(delta.y)) {
                if(abs(delta.x) < abs(delta.z)) {
                    // x
                    closestPoint = vec3(delta.x > 0 ? halfSize.x : -halfSize.x,closestPoint.y,closestPoint.z);
                } else {
                    // z
                    closestPoint = vec3(closestPoint.x,closestPoint.y,delta.z > 0 ? halfSize.z : -halfSize.z);
                }
            } else {
                if(abs(delta.y) < abs(delta.z)) {
                    // y
                    closestPoint = vec3(closestPoint.x,delta.y > 0 ? halfSize.y : -halfSize.y,closestPoint.z);
                } else {
                    // z
                    closestPoint = vec3(closestPoint.x,closestPoint.y,delta.z > 0 ? halfSize.z : -halfSize.z);
                }
            }
            deltaToPoint = closestPoint - delta;
            deltaToPointLength = glm::length(deltaToPoint); //recalculate
            penetration = deltaToPointLength + radius;
        }
        if(penetration > 0) {
            return Contact(closestPoint + boxPos,glm::normalize(deltaToPoint),penetration);
        } else {
            return std::nullopt;
        }
    }

    std::optional<Contact> intersectSphereTri(vec3 spherePos,float radius,vec3 a,vec3 b,vec3 c) {

        Ray plane = Ray(a,MathHelper::normalFromPlanePoints(a,b,c));
        float distanceToPlane = glm::dot(plane.direction,(spherePos - plane.origin));
        vec3 deltaFromPlane = (plane.direction * distanceToPlane);

        vec3 pointOnPlane = spherePos - deltaFromPlane;
        
        bool insideTri = true;
        //test if its on the correct side of each line
        if(glm::dot(plane.direction,glm::cross(b-a,pointOnPlane-a)) < 0) {
            insideTri = false;
        }
        else if(glm::dot(plane.direction,glm::cross(c-b,pointOnPlane-b)) < 0) {
            insideTri = false;
        }
        else if(glm::dot(plane.direction,glm::cross(a-c,pointOnPlane-c)) < 0) {
            insideTri = false;
        }

        vec3 deltaFromTri;
        if(insideTri) {
            deltaFromTri = deltaFromPlane;
        } else {

            vec3 ab = spherePos - glm::closestPointOnLine(spherePos,a,b);
            vec3 bc = spherePos - glm::closestPointOnLine(spherePos,b,c);
            vec3 ca = spherePos - glm::closestPointOnLine(spherePos,c,a);
            if(glm::length2(ab) < glm::length2(bc)) {
                if(glm::length2(ab) < glm::length2(ca)) {
                    deltaFromTri = ab;
                } else {
                    deltaFromTri = ca;
                }
            } else {
                if(glm::length2(bc) < glm::length2(ca)) {
                    deltaFromTri = bc;
                } else {
                    deltaFromTri = ca;
                }
            }

        }
        float penetration = radius - glm::length(deltaFromTri);
        
        if(penetration > 0) {
            return Contact(spherePos - deltaFromTri,glm::normalize(deltaFromTri),penetration);
        } else {
            return std::nullopt;
        }
        

    }


};