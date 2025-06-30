#pragma once

#include <engine/world.hpp>
#include <actor/construction.hpp>

using glm::vec3,glm::quat,glm::mat4;

class Tool {

    public:
        Model* heldModel;
        Material* heldModelMaterial;
        vec3 modelOffset;
        quat modelRotation;
        quat lookOrientation = glm::identity<quat>();
        float modelScale;
        float lookLerp;
        virtual void use(World* world,vec3 position,vec3 direction) = 0;

        virtual void render(Camera& camera) {
            if(heldModel != nullptr && heldModelMaterial != nullptr) {
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,modelOffset);
                matrix = matrix * glm::toMat4(modelRotation);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->render(camera.getViewRotationMatrix(),glm::toMat4(glm::inverse(lookOrientation)) * matrix,camera.getProjectionMatrix(),*heldModelMaterial);
            }
        }

        virtual void setLookOrientation(quat rotation) {
            lookOrientation = rotation;
        }

        virtual void lerpLookOrientation(quat rotation,float dt) {
            lookOrientation = glm::slerp(lookOrientation,rotation,lookLerp * dt);
        }

};