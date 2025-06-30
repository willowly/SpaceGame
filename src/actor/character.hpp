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
#include "rigidbody-actor.hpp"
#include "engine/input.hpp"
#include "engine/world.hpp"
#include "construction.hpp"

#include <item/pickaxe-tool.hpp>
#include <item/place-block-tool.hpp>

#include <GLFW/glfw3.h>


class Character : public RigidbodyActor {
    public: 

        Character() : Character(nullptr,nullptr) {

        }
        Character(Model* model,Material* material) : RigidbodyActor(model,material) {
        }
        float moveSpeed = 5.0f;
        float lookPitch = 0;
        float lookSensitivity = 5;
        float height = 0.75;
        float radius = 0.5;
        float acceleration = 10;
        float jumpForce = 10;
        bool clickInput = false;
        bool interactInput = false;

        vec3 moveInput;

        Tool* currentTool = nullptr;

        PlaceBlockTool placeTin;
        PlaceBlockTool placeCobalt;
        PlaceBlockTool placeChair;
        PlaceBlockTool placeThruster;
        PickaxeTool pickaxe;

        Construction* ridingConstruction = nullptr;
        ivec3 ridingConstructionPoint;

        vec3 thirdPersonCameraOffset = vec3(0,3,20);
        //vec3 thirdPersonCameraRot;


        void render(Camera& camera,float dt) {
            if(currentTool != nullptr && ridingConstruction == nullptr) {
                currentTool->lerpLookOrientation(getEyeRotation(),dt);
                currentTool->render(camera);
            }
        }

        void step(float dt,World* world) {

            float moveSpeed = 5.0f;
            
            

            if(interactInput) {
                interact(world);
            }

            if(ridingConstruction != nullptr) {
                ridingConstruction->setTargetVelocity(moveInput * 100.0f);
                ridingConstruction->setTargetRotation(glm::inverse(getEyeRotation()));
                position = ridingConstruction->transformPoint(ridingConstructionPoint); 
            } else {
                if(clickInput) {
                    if(currentTool != nullptr) {
                        currentTool->use(world,getEyePosition(),getEyeDirection());
                    }
                }
                vec3 targetVelocity = glm::inverse(rotation) * moveInput * moveSpeed;
                velocity = MathHelper::lerp(velocity,targetVelocity,acceleration*dt);
            }

            

            RigidbodyActor::step(dt,world);

            clickInput = false;
            interactInput = false;

        }

        void ride(Construction* construction,ivec3 point) {
            if(ridingConstruction != nullptr) {
                dismount();
            }
            ridingConstruction = construction;
            ridingConstructionPoint = point;
            body->getCollider(0)->setIsTrigger(true);
        }

        void dismount() {
            ridingConstruction->resetTargets();
            ridingConstruction = nullptr;
            body->getCollider(0)->setIsTrigger(false);
        }

        void interact(World* world) {
            if(ridingConstruction != nullptr) {
                dismount();
                return;
            }
            std::cout << "interacting with world" << std::endl;
            ph::RaycastCallback callback;
            world->raycast(getEyePosition(),getEyeDirection(),10,&callback);
            if(callback.success) {
                ActorUserData* data = static_cast<ActorUserData*>(callback.body->getUserData());
                Construction* construction = dynamic_cast<Construction*>(data->actor);
                if(construction != nullptr) {
                    std::cout << "interacted with construction" << std::endl;
                    vec3 interactPointWorld = callback.worldPoint - callback.worldNormal * 0.5f;
                    vec3 interactPointLocal = construction->inverseTransformPoint(interactPointWorld);
                    ivec3 interactPointInt = glm::round(interactPointLocal);
                    Block* block = construction->getBlock(interactPointInt);
                    if(block != nullptr) {
                        std::cout << "interacted with block" << StringHelper::toString(interactPointInt) << std::endl;
                        if(block->canRide) {
                            ride(construction,interactPointInt);
                        }
                    }
                }
            }
        }

        //could layer be abstracted to a controller
        void processInput(Input& input) {

            // eventually we want an enum for keys instead of using the defines
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

            if(input.getKeyPressed(GLFW_KEY_E)) {
                interactInput = true;
            }

            if(input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1)) {
                clickInput = true;
            }

            if(input.getKeyPressed(GLFW_KEY_1)) {
                currentTool = &placeTin;
                currentTool->setLookOrientation(getEyeRotation());
            }
            if(input.getKeyPressed(GLFW_KEY_2)) {
                currentTool = &placeCobalt;
                currentTool->setLookOrientation(getEyeRotation());
            }
            if(input.getKeyPressed(GLFW_KEY_3)) {
                currentTool = &placeChair;
                currentTool->setLookOrientation(getEyeRotation());
            }
            if(input.getKeyPressed(GLFW_KEY_4)) {
                currentTool = &placeThruster;
                currentTool->setLookOrientation(getEyeRotation());
            }
            if(input.getKeyPressed(GLFW_KEY_5)) {
                currentTool = &pickaxe;
                currentTool->setLookOrientation(getEyeRotation());
            }

            moveMouse(input.getMouseDelta() * 0.01f);
        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,vec3(0,1,0)) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void setCamera(Camera& camera) {
            if(ridingConstruction == nullptr) {
                camera.position = getEyePosition();
                camera.rotation = getEyeRotation();
            } else {
                camera.position = position + glm::inverse(getEyeRotation()) * thirdPersonCameraOffset;
                camera.rotation = getEyeRotation();
            }
            
        }

        vec3 getEyePosition() {
            return position + vec3(0,height,0);
        }

        quat getEyeRotation() {
            return glm::angleAxis(glm::radians(lookPitch),vec3(1,0,0)) * rotation;
        }

        vec3 getEyeDirection() {
            return vec3(0,0,-1) * getEyeRotation();
        }

        bool playerStep() {
            return true;
        }

        void addToPhysicsWorld(rp3d::PhysicsWorld* world,rp3d::PhysicsCommon* common) {
            RigidbodyActor::addToPhysicsWorld(world,common);
            body->setAngularLockAxisFactor(rp3d::Vector3(0,0,0));
            body->enableGravity(false);
        }

        void addCollisionShapes(rp3d::PhysicsCommon* common) {
            auto collider = body->addCollider(common->createCapsuleShape(radius,height),rp3d::Transform(rp3d::Vector3(0,height/2,0),rp3d::Quaternion::identity()));
            collider->getMaterial().setBounciness(0.0);
        }

};