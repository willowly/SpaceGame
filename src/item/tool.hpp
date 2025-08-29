#pragma once

#include <engine/world.hpp>
#include <actor/construction.hpp>
#include "tool-user.hpp"
#include "engine/input.hpp"

using glm::vec3,glm::quat,glm::mat4;

class Character;

class Tool {

    public:
        Texture* icon;
        Model* heldModel = nullptr;
        Material* heldModelMaterial = nullptr;
        vec3 modelOffset = vec3(0.3,-0.3,-1);
        quat modelRotation = quat(vec3(0,glm::radians(45.0f),0));
        quat lookOrientation = glm::identity<quat>(); //the rendered one, that gets lerped
        float modelScale =  0.3f;
        float lookLerp = 10;

        bool clickInput = false;

        Tool() {

        }
        Tool(Model* heldModel,Material* heldModelMaterial,vec3 modelOffset,quat modelRotation,float modelScale,float lookLerp) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset),modelRotation(modelRotation),modelScale(modelScale),lookLerp(lookLerp) {

        }
        Tool(Model* heldModel,Material* heldModelMaterial,vec3 modelOffset,quat modelRotation) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset), modelRotation(modelRotation) {
            
        }
        Tool(Model* heldModel,Material* heldModelMaterial) : heldModel(heldModel), heldModelMaterial(heldModelMaterial) {
            
        }

        virtual ~Tool() {}


        virtual void equip(ToolUser* user) {
            lookOrientation = user->getEyeRotation();
        }

        virtual void unequip(ToolUser* user) {

        }

        virtual void processInput(Input& input) {
            if(input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1)) {
                clickInput = true;
            }
        }

        virtual void step(World* world,ToolUser* user,float dt) {

        }

        virtual quat getAnimationRotation() {

        }

        virtual void render(Camera& camera,ToolUser* user,float dt) {
            if(heldModel != nullptr && heldModelMaterial != nullptr) {
                lookOrientation = glm::slerp(lookOrientation,user->getEyeRotation(),lookLerp * dt);
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,modelOffset);
                matrix = matrix * glm::toMat4(modelRotation);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->render(camera.getViewRotationMatrix(),glm::toMat4(lookOrientation) * matrix,camera.getProjectionMatrix(),*heldModelMaterial);
            }
        }

};