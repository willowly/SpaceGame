#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"

using glm::vec3;

class GravityWell {
    

    
        vec3 position = {};
        float radius = 40;
        float mass = 0;

    public:

        GravityWell() {}
        GravityWell(vec3 position,float surfaceAcceleration,float radius) : position(position), mass(radius*radius*surfaceAcceleration),radius() {}

        vec3 getGravityVector(vec3 position) {
            vec3 delta = this->position - position;
            float distance2 = glm::length2(delta);

            return glm::normalize(delta) * mass/distance2;
        }

};