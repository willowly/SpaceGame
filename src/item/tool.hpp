#pragma once

#include <engine/world.hpp>
#include <actor/construction.hpp>
#include "tool-user.hpp"
#include "engine/input.hpp"

using glm::vec3,glm::quat,glm::mat4;

class Character;

class Tool {

    public:
        TextureID icon;
        Model* heldModel = nullptr;
        Material heldModelMaterial;
        vec3 modelOffset = vec3(0.3,-0.3,-1);
        quat modelRotation = quat(vec3(0,glm::radians(45.0f),0));
        quat lookOrientation = glm::identity<quat>(); //the rendered one, that gets lerped
        float modelScale =  0.3f;
        float lookLerp = 10;

        bool clickInput = false;
        bool clickHold = false;

        Tool() {

        }
        Tool(Model* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation,float modelScale,float lookLerp) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset),modelRotation(modelRotation),modelScale(modelScale),lookLerp(lookLerp) {

        }
        Tool(Model* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset), modelRotation(modelRotation) {
            
        }
        Tool(Model* heldModel,Material heldModelMaterial) : heldModel(heldModel), heldModelMaterial(heldModelMaterial) {
            
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
            clickHold = input.getMouseButton(GLFW_MOUSE_BUTTON_1);
        }

        virtual void step(World* world,ToolUser* user,float dt) {

        }

        virtual std::pair<quat,vec3> animate(float dt) {
            return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
        }

        virtual void addRenderables(Vulkan* vulkan,ToolUser* user,float dt) {
            if(heldModel != nullptr) {
                lookOrientation = glm::slerp(lookOrientation,user->getEyeRotation(),lookLerp * dt);
                auto animation = animate(dt);
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,modelOffset + animation.second);
                matrix = matrix * glm::toMat4(modelRotation * animation.first);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->addToRender(vulkan,heldModelMaterial,user->getTransform() * matrix);
            }
        }

};