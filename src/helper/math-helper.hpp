#include <include/glm/glm.hpp>

using glm::vec3;

namespace MathHelper {
    vec3 lerp(vec3 a,vec3 b,float t) {
        return b*t + a*(1.0f-t);
    }
}