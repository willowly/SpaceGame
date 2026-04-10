#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

struct data_vec3 {
    float x;
    float y;
    float z;

    glm::vec3 toVec3() const {
        return glm::vec3(x,y,z);
    }

    void set(glm::vec3 v) {
        x = v.x;
        y = v.y;
        z = v.z;
    }
};

struct data_ivec3 {
    int x;
    int y;
    int z;

    glm::ivec3 toVec3() const {
        return glm::ivec3(x,y,z);
    }

    void set(glm::ivec3 v) {
        x = v.x;
        y = v.y;
        z = v.z;
    }
};

struct data_quat {
    float x;
    float y;
    float z;
    float w;

    glm::quat toQuat() const {
        return glm::quat(w,x,y,z);
    }

    void set(glm::quat q) {
        x = q.x;
        y = q.y;
        z = q.z;
        w = q.w;
    }


};