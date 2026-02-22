#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using glm::vec3,glm::quat;

class CameraShake {


    private:
        float timer = -1;
        vec3 rotation;

        vec3 altRotation;
        float altOffset;


    public:
        vec3 rotationMultiplier = vec3(1.0f);
        vec3 altRotationMultiplier = vec3(0.4f); //relative to main rotation
        float frequency = 5;
        float altFrequencyMultiplier = 2.0f;
        float tiltAmount = 1;
        float time = 0.5f;


        quat getRotation() {
            if(timer < 0 || timer > time) return glm::identity<quat>();
            vec3 out = rotation * rotationMultiplier;
            out *= glm::sin(timer * frequency * 3.14); // * pi so that its proper cycles/second

            vec3 alt = altRotation * rotationMultiplier * altRotationMultiplier;
            alt *= glm::sin((timer + altOffset) * frequency * altFrequencyMultiplier * 3.14);

            out += alt;
            out *= (1-(timer/time));
            return quat(glm::radians(out));
        }

        void startShake() {
            rotation = vec3(glm::circularRand(1.0f),glm::linearRand(-tiltAmount,tiltAmount));
            altRotation = vec3(glm::circularRand(1.0f),0.0f);
            altOffset = glm::linearRand(0.3f,0.8f);
            timer = 0.0f;
        }

        void step(float dt) {
            if(timer < 0) return;
            timer += dt;
        }

    


    


};