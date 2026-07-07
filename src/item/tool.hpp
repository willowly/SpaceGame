#pragma once

#include <engine/world.hpp>
#include "engine/input.hpp"
#include "actor/character.hpp"
#include "item.hpp"

#include "helper/event.hpp"

using glm::vec3,glm::quat,glm::mat4;

class Tool : public Item {

    public:
        Mesh<Vertex>* heldModel = nullptr;
        MaterialObject* heldModelMaterial = nullptr;
        vec3 modelOffset = vec3(0.3,-0.3,-1);
        quat modelRotation = quat(vec3(0,glm::radians(45.0f),0));
        float modelScale =  0.3f;
        float smoothTime = 0.01f; //lower value is faster

        Tool() {

        }

        virtual ~Tool() {}


        virtual void equip(Character& user) {
            user.heldItemData.lookOrientation = user.getEyeRotation();
        }

        virtual void unequip(Character& user) {
            
        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

        }

        void sendActionEvent(Character& user,int actionEvent) {
            user.onToolAction({&user,this,actionEvent});
        }

        virtual std::pair<quat,vec3> animate(Character& user,float dt) {
            return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
        }

        virtual void addRenderablesHeld(Vulkan* vulkan,Character& user,float dt,float interpolation) override {
            if(heldModel != nullptr) {
                if(heldModelMaterial == nullptr) return;
                user.heldItemData.lookOrientation = glm::slerp(user.heldItemData.lookOrientation,user.getEyeRotation(),1 - pow(smoothTime,dt));
                auto animation = animate(user,dt);
                auto matrix = glm::mat4(1.0f);
                matrix = glm::translate(matrix,user.getEyePositionInterpolated(interpolation));
                matrix = matrix * glm::toMat4(user.heldItemData.lookOrientation);
                matrix = glm::translate(matrix,modelOffset + animation.second);
                matrix = matrix * glm::toMat4(modelRotation * animation.first);
                matrix = glm::scale(matrix,vec3(modelScale));
                heldModel->addToRender(vulkan,heldModelMaterial->material,matrix);
            }
        }

        virtual string getTypeName() {
            return "tool";
        }

};