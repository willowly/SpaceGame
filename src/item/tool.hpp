#pragma once

#include <engine/world.hpp>
#include "engine/input.hpp"
#include "actor/character.hpp"
#include "item.hpp"

using glm::vec3,glm::quat,glm::mat4;

class Tool : public Item {

    public:
        Mesh<Vertex>* heldModel = nullptr;
        Material heldModelMaterial = Material::none;
        vec3 modelOffset = vec3(0.3,-0.3,-1);
        quat modelRotation = quat(vec3(0,glm::radians(45.0f),0));
        quat lookOrientation = glm::identity<quat>(); //the rendered one, that gets lerped
        float modelScale =  0.3f;
        float smoothTime = 0.01f; //lower value is faster
    
        bool clickInput = false;
        bool clickHold = false;

        Tool() {

        }
        Tool(Mesh<Vertex>* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation,float modelScale,float lookLerp) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset),modelRotation(modelRotation),modelScale(modelScale),smoothTime(smoothTime) {

        }
        Tool(Mesh<Vertex>* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation) : heldModel(heldModel), heldModelMaterial(heldModelMaterial), modelOffset(modelOffset), modelRotation(modelRotation) {
            
        }
        Tool(Mesh<Vertex>* heldModel,Material heldModelMaterial) : heldModel(heldModel), heldModelMaterial(heldModelMaterial) {
            
        }

        virtual ~Tool() {}


        virtual void equip(Character& user) {
            lookOrientation = user.getEyeRotation();
        }

        virtual void unequip(Character& user) {
            
        }


        virtual void processInput(Input& input) {
            if(input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1)) {
                clickInput = true;
            }
            clickHold = input.getMouseButton(GLFW_MOUSE_BUTTON_1);
        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

        }

        virtual std::pair<quat,vec3> animate(Character& user,float dt) {
            return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
        }

        virtual void addRenderablesHeld(Vulkan* vulkan,Character& user,float dt,float interpolation) override {
            if(heldModel != nullptr) {
                lookOrientation = glm::slerp(lookOrientation,user.getEyeRotation(),1 - pow(smoothTime,dt));
                auto animation = animate(user,dt);
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,user.getEyePositionInterpolated(interpolation));
                matrix = matrix * glm::toMat4(lookOrientation);
                matrix = glm::translate(matrix,modelOffset + animation.second);
                matrix = matrix * glm::toMat4(modelRotation * animation.first);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->addToRender(vulkan,heldModelMaterial,matrix);
            }
        }

};