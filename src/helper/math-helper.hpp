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