#pragma once
#include <cstdlib>

#include "glm/glm.hpp"
#include "glm/gtc/random.hpp"
#include "glm/gtx/quaternion.hpp"

namespace Random {

    // generates a value between 0 and -1
    float value() {
        return glm::linearRand(0.0f,1.0f);
    }

    // generates a value between -x and +x
    template<typename T>
    T box(T x) {
        return glm::linearRand(-x,x);
    }

    template<typename T>
    T linear(T min,T max) {
        return glm::linearRand(min,max);
    }

    glm::quat rotation() {
        return glm::quat(glm::radians(box(vec3(360))));
    }
    
}
