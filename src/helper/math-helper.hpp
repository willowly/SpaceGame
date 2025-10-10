#pragma once
#include <glm/glm.hpp>

using glm::vec3;

namespace MathHelper {

    
    vec3 lerp(vec3 a,vec3 b,float t) {
        return b*t + a*(1.0f-t);
    }

    float lerp(float a,float b,float t) {
        return b*t + a*(1.0f-t);
    }

    vec3 clampLength(vec3 a,float length) {
        return glm::normalize(a) * glm::min((float)a.length(),length);
    }

    vec3 moveTowards(vec3 a,vec3 b,float maxDelta) {
        vec3 delta = b - a;
        return a + clampLength(delta,maxDelta);
    }

    float sign(float a) {
        if(a < 0) {
            return -1;
        } else {
            return 1;
        }
    }

    vec3 normalFromPlanePoints(vec3 a,vec3 b,vec3 c) {
        return glm::normalize(glm::cross(b-a,c-a));
    }

    vec3 closestPointOnLineSegment(vec3 p,vec3 a, vec3 b) {
        vec3 a2b = b-a;
        vec3 a2p = p-a;
        float onLine = glm::dot(a2p,glm::normalize(a2b));
        onLine = std::clamp(onLine,0.0f,glm::length(a2b));
        return (a2b*onLine) + a;
    }

    //
    float integerBelow(float a) {
        if(floor(a) == a) {
            return a - 1;
        }
        return floor(a);
    }

    float fromFloor(float a) {
        return a - floor(a);
    }

    glm::mat4 getTransformMatrix(vec3 position,quat rotation = quat(1.0f,0.0f,0.0f,0.0f),vec3 scale = vec3(1)) {
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