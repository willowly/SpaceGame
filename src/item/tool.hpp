#pragma once

#include <engine/world.hpp>
#include <actor/construction.hpp>

using glm::vec3,glm::quat,glm::mat4;

class Tool {

    public:
        Texture* icon;
        Model* heldModel = nullptr;
        Material* heldModelMaterial = nullptr;
        vec3 modelOffset = vec3(0.3,-0.3,-1);
        quat modelRotation = quat(vec3(0,glm::radians(45.0f),0));
        quat lookOrientation = glm::identity<quat>();
        float modelScale =  0.3f;
        float lookLerp = 10;

        Tool() {

        }
        Tool(Model* heldModel,Material* heldModelMaterial,vec3 modelOffset,quat modelRotation,float modelScale,float lookLerp) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset),modelRotation(modelRotation),modelScale(modelScale),lookLerp(lookLerp) {

        }
        Tool(Model* heldModel,Material* heldModelMaterial,vec3 modelOffset,quat modelRotation) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset), modelRotation(modelRotation) {
            
        }
        Tool(Model* heldModel,Material* heldModelMaterial) : heldModel(heldModel), heldModelMaterial(heldModelMaterial) {
            
        }

        virtual ~Tool() {}


        virtual void use(World* world,vec3 position,vec3 direction) = 0;

        virtual void render(Camera& camera) {
            if(heldModel != nullptr && heldModelMaterial != nullptr) {
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,modelOffset);
                matrix = matrix * glm::toMat4(modelRotation);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->render(camera.getViewRotationMatrix(),glm::toMat4(lookOrientation) * matrix,camera.getProjectionMatrix(),*heldModelMaterial);
            }
        }

        virtual void setLookOrientation(quat rotation) {
            lookOrientation = rotation;
        }

        virtual void lerpLookOrientation(quat rotation,float dt) {
            lookOrientation = glm::slerp(lookOrientation,rotation,lookLerp * dt);
        }

};