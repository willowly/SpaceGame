#pragma once
#include "actor.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>
#include <string>
#include <iostream>
#include "helper/math-helper.hpp"
#include "helper/string-helper.hpp"
//#include "rigidbody-actor.hpp"
#include "engine/input.hpp"
#include "engine/world.hpp"
#include "construction.hpp"

#include "item/item-stack.hpp"
#include "item/recipe.hpp"
#include "components/inventory.hpp"

#include <GLFW/glfw3.h>


#include "interface/actor/actor-widget.hpp"
#include "interface/i-has-menu.hpp"


class Character : public Actor {


    public: 

        // Prototype constructors
        
        // enum Action {
        //     Neutral,
        //     InMenu
        // } action = Action::Neutral;


        float moveSpeed = 5.0f;
        float lookPitch = 0;
        float lookSensitivity = 5;
        float height = 0.4f;
        float radius = 1;
        float acceleration = 10;
        float jumpForce = 10;
        bool clickInput = false;
        bool interactInput = false;

        bool thirdPerson = true;

        vec3 moveInput;

        vec3 velocity;

        Item* currentToolItem;
        int selectedTool;

        Item* toolbar[9] = {};

        struct HeldItemData {
            float actionTimer;
            float animationTimer;
            int action;
            void setAction(int newAction) {
                action = newAction;
                actionTimer = 0;
                animationTimer = 0;
            }
        } heldItemData;

        Construction* ridingConstruction = nullptr;
        ivec3 ridingConstructionPoint;
        quat ridingConstructionRotation;

        vec3 thirdPersonCameraOffset = vec3(0,3,20);
        vec3 thirdPersonCameraRot;

        Inventory inventory;

        IHasMenu* openMenuObject;

        std::vector<Recipe*> recipes;

        bool inMenu;

        ActorWidget<Character>* widget;


        void addRenderables(Vulkan* vulkan,float dt) {
            ItemStack* currentTool = inventory.getStack(currentToolItem);
            if(currentTool != nullptr && ridingConstruction == nullptr) {
                heldItemData.animationTimer += dt;
                currentTool->item->addRenderables(vulkan,*this,dt);
            }
            if(thirdPerson) {
                Actor::addRenderables(vulkan,dt);
            }
        }

        void step(World* world,float dt) {

            

            if(ridingConstruction != nullptr) {
                ridingConstruction->setMoveControl(ridingConstructionRotation * moveInput);
                ridingConstruction->setTargetRotation(getEyeRotation() * glm::inverse(ridingConstructionRotation) * glm::angleAxis(glm::radians(180.0f),vec3(0,1,0)));
                position = ridingConstruction->transformPoint(ridingConstructionPoint);
            } else {
                vec3 targetVelocity = rotation * moveInput * moveSpeed;
                velocity = MathHelper::lerp(velocity,targetVelocity,acceleration*dt);
                position += velocity * dt;

                if(!inMenu) {
                    if(interactInput) {
                        interact(world);
                    }
                    ItemStack* currentTool = inventory.getStack(currentToolItem);
                    if(currentTool != nullptr) {
                        currentTool->item->step(world,*this,*currentTool,dt);
                        heldItemData.actionTimer += dt;
                    }
                }

                clickInput = false;
                interactInput = false;

                world->collideBasic(this,radius);
            }

            

        }

        void ride(Construction* construction,ivec3 point,quat rotation) {
            if(ridingConstruction != nullptr) {
                dismount();
            }
            ridingConstruction = construction;
            ridingConstructionPoint = point;
            ridingConstructionRotation = rotation;
            //body->getCollider(0)->setIsTrigger(true);
        }

        void dismount() {
            position += ridingConstruction->transformDirection(vec3(0,1,0));
            ridingConstruction->resetTargets();
            ridingConstruction = nullptr;
            //body->getCollider(0)->setIsSimulationCollider(true);
        }

        void interact(World* world) {
            if(ridingConstruction != nullptr) {
                dismount();
                return;
            }
            auto worldHitOpt = world->raycast(Ray(getEyePosition(),getEyeDirection()),10);
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    std::cout << "interacted with construction" << std::endl;
                    vec3 interactPointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 interactPointLocal = construction->inverseTransformPoint(interactPointWorld);
                    ivec3 interactPointInt = glm::round(interactPointLocal);
                    auto data = construction->getBlock(interactPointInt);
                    auto block = data.first;
                    auto state = data.second;
                    if(data.first != nullptr) {
                        block->onInteract(construction,interactPointInt,state,*this);
                    }
                }
            }
        }

        //could layer be abstracted to a controller
        void processInput(Input& input) {

            // eventually we want an enum for keys instead of using the defines
            

            if(inMenu) {
                processInputInventory(input);
            } else {
                processInputNormal(input);
            }
        }

        void processInputNormal(Input& input) {
            moveInput = vec3(0,0,0);
            if(input.getKey(GLFW_KEY_W)) {
                moveInput.z -= 1; //im not sure why this exists :shrug:
            }
            if(input.getKey(GLFW_KEY_S)) {
                moveInput.z += 1;
            }
            if(input.getKey(GLFW_KEY_A)) {
                moveInput.x -= 1;
            }
            if(input.getKey(GLFW_KEY_D)) {
                moveInput.x += 1;
            }
            if(input.getKey(GLFW_KEY_SPACE)) {
                moveInput.y += 1;
            }
            if(input.getKey(GLFW_KEY_C)) {
                moveInput.y -= 1;
            }

            if(input.getKeyPressed(GLFW_KEY_F)) {
                interactInput = true;
            }

            if(input.getKeyPressed(GLFW_KEY_1)) {
                setCurrentTool(0);
            }
            if(input.getKeyPressed(GLFW_KEY_2)) {
                setCurrentTool(1);
            }
            if(input.getKeyPressed(GLFW_KEY_3)) {
                setCurrentTool(2);
            }
            if(input.getKeyPressed(GLFW_KEY_4)) {
                setCurrentTool(3);
            }
            if(input.getKeyPressed(GLFW_KEY_5)) {
                setCurrentTool(4);
            }
            if(input.getKeyPressed(GLFW_KEY_6)) {
                setCurrentTool(5);
            }
            if(input.getKeyPressed(GLFW_KEY_7)) {
                setCurrentTool(6);
            }
            if(input.getKeyPressed(GLFW_KEY_8)) {
                setCurrentTool(7);
            }
            if(input.getKeyPressed(GLFW_KEY_9)) {
                setCurrentTool(8);
            }
            if(input.getKeyPressed(GLFW_KEY_Z)) {
                thirdPerson = !thirdPerson;
            }

            ItemStack* currentTool = inventory.getStack(currentToolItem);
            if(currentTool != nullptr) {
                currentTool->item->processInput(input);
            }

            if(input.getKeyPressed(GLFW_KEY_TAB)) {
                openMenu();
            }
            moveMouse(input.getMouseDelta() * 0.01f);
        }

        void processInputInventory(Input& input) {
            moveInput = vec3(0,0,0);
            if(input.getKeyPressed(GLFW_KEY_TAB)) {
                closeMenu();
            }
            if(input.getKeyPressed(GLFW_KEY_F)) {
                closeMenu();
            }
            input.getMouseDelta();
        }

        void setCurrentTool(int index) {

            auto newTool = inventory.getStack(toolbar[index]);
            auto currentTool = inventory.getStack(currentToolItem);
            if(newTool == currentTool) return; //dont do anything if its the same tool

            if(currentTool != nullptr) {
                currentTool->item->unequip(*this);
            }
            selectedTool = index;
            if(newTool != nullptr) {
                newTool->item->equip(*this);
                heldItemData.setAction(0); // reset actions
                currentToolItem = newTool->item;
            } else {
                currentToolItem = nullptr;
            }
            
            
        }

        void setToolbar(int index,Item* item) {
            for(auto&& itemInBar : toolbar) {
                if(itemInBar == item) {
                    itemInBar = nullptr;
                }
            }
            toolbar[index] = item;
        }

        // for tools to do when they reduce count etc
        void refreshTool() {
            setCurrentTool(selectedTool);
        }

        void closeMenu() {
            inMenu = false;
            openMenuObject = nullptr;
        }

        void openMenu() {
            openMenu(nullptr);
        }

        void openMenu(IHasMenu* menuObject) {
            heldItemData.setAction(0);
            inMenu = true;
            openMenuObject = menuObject;
            
        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,vec3(0,-1,0)) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void setCamera(Camera& camera) {
            // if(ridingConstruction == nullptr && !thirdPerson) {
                 camera.position = getEyePosition();
                 camera.rotation = getEyeRotation();
            // } else {
                // camera.position = position + getEyeRotation() * thirdPersonCameraOffset;
                // camera.rotation = getEyeRotation();
            //}
            
        }

        static std::unique_ptr<Character> makeDefaultPrototype() {
            auto ptr = new Character();
            return std::unique_ptr<Character>(ptr);
        }

        static std::unique_ptr<Character> makeInstance(Character* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            return makeInstanceFromPrototype<Character>(prototype,position,rotation);
        }

        glm::mat4 getTransform() {
            return Actor::getTransform();
        }

        vec3 getEyePosition() {
            return position + vec3(0,height,0);
        }

        quat getEyeRotation() {
            return rotation * glm::angleAxis(glm::radians(lookPitch),vec3(-1,0,0));
        }

        vec3 getEyeDirection() {
            return getEyeRotation() * vec3(0,0,-1);
        }

        Ray getLookRay() {
            return Ray(getEyePosition(),getEyeDirection());
        }

        bool playerStep() {
            return true;
        }

    protected:
        Character() : Actor() {
            
        }

};