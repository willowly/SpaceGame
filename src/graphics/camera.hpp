#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using glm::vec2,glm::vec3,glm::quat;

class Camera {

    public:
        vec3 position = vec3(0);
        quat rotation = quat(1,0,0,0);
        // float pitch;
        // float yaw;
        float fov = 45.0f;
        float nearClip = 0.1f;
        float farClip = 1000.0f;
        float aspect;

        void move(vec3 motion) {
            position += motion;
        }

        void rotate(vec3 rotationAmount) {
            rotation *= glm::angleAxis(glm::radians(rotationAmount.x),vec3(1,0,0));
            rotation *= glm::angleAxis(glm::radians(rotationAmount.y),vec3(0,1,0));
            rotation *= glm::angleAxis(glm::radians(rotationAmount.z),vec3(0,0,1));
        }

        vec3 direction() {
            return vec3(0,0,-1) * rotation;
        }

        void setAspect(float x,float y) {
            this->aspect = x/y;
        }

        glm::mat4 getViewMatrix() {
            glm::mat4 view = glm::mat4(1.0f);
            view = glm::toMat4(glm::inverse(rotation)) * view;
            view = glm::translate(view,-position);
            return view;
        }

        glm::mat4 getViewRotationMatrix() {
            glm::mat4 view = glm::mat4(1.0f);
            view = glm::toMat4(glm::inverse(rotation)) * view;
            return view;
        }

        glm::mat4 getProjectionMatrix() {
            return glm::perspective(glm::radians(fov), aspect, nearClip, farClip);
        }

};