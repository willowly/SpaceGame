#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <algorithm>

using glm::vec3;

#define _USE_MATH_DEFINES // need this for constants (M_E)
#include <cmath> // for constants

namespace MathHelper {

    
    inline vec3 lerp(vec3 a,vec3 b,float t) {
        return b*t + a*(1.0f-t);
    }


    inline float lerp(float a,float b,float t) {
        return b*t + a*(1.0f-t);
    }

    inline vec3 clampLength(vec3 a,float length) {
        auto currentLength = glm::length(a);
        if(currentLength < 1.0E-9) {
            return a;
        }
        return glm::normalize(a) * glm::min(currentLength,length);
    }

    inline vec3 moveTowards(vec3 a,vec3 b,float maxDelta) {
        vec3 delta = b - a;
        return a + clampLength(delta,maxDelta);
    }

    inline float moveTowards(float a,float b,float maxDelta) {
        float delta = b - a;
        if(delta > maxDelta) {
            delta = maxDelta;
        }
        return a + delta;
    }

    inline float sign(float a) {
        if(a < 0) {
            return -1;
        } else {
            return 1;
        }
    }

    inline vec3 normalFromPlanePoints(vec3 a,vec3 b,vec3 c) {
        return glm::normalize(glm::cross(b-a,c-a));
    }

    inline vec3 closestPointOnLineSegment(vec3 p,vec3 a, vec3 b) {
        vec3 a2b = b-a;
        vec3 a2p = p-a;
        float onLine = glm::dot(a2p,glm::normalize(a2b));
        onLine = std::clamp(onLine,0.0f,glm::length(a2b));
        return (a2b*onLine) + a;
    }

    // returns the acceleration
    inline float accelerateTo(float current,float target,float velocity,float maxAcceleration,float dt) {
        
    }

    // inline float smoothDamp(float current,float target,float& velocity,float smoothTime,float dt) {
    //     auto w = 2 / smoothTime;
    //     float next = target + ((current-target) + (velocity + w*(current - target))*dt)*std::powf(M_E,-w*dt);
    //     velocity = 
    // }

    //
    inline float integerBelow(float a) {
        if(floor(a) == a) {
            return a - 1;
        }
        return floor(a);
    }

    inline float fromFloor(float a) {
        return a - floor(a);
    }

    inline glm::mat4 getTransformMatrix(vec3 position,quat rotation = quat(1.0f,0.0f,0.0f,0.0f),vec3 scale = vec3(1)) {
        auto matrix = glm::translate(glm::mat4(1.0f),position);
        matrix = matrix * glm::toMat4(rotation);
        matrix = glm::scale(matrix,scale);
        return matrix;
    }
}

template <>
class std::less<ivec3> {

    bool operator()(const ivec3& a, const ivec3& b) const
    {
        if(a.x == b.x) {
            if(a.y == b.y) {
                return a.z < b.z;
            }
            return a.y < b.y;
        }
        return a.x < b.x;
    }
};

namespace mh = MathHelper;