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

#include <GLFW/glfw3.h>


class Character : public RigidbodyActor {
    public: 

        Character() {

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

        vec3 moveInput;

        void step(float dt,World* world) {

            float moveSpeed = 5.0f;
            
            vec3 targetVelocity = glm::inverse(rotation) * moveInput * moveSpeed;
            //vec3 moveVelocity = vec3(velocity.x,0,velocity.z);
            velocity = MathHelper::lerp(velocity,targetVelocity,acceleration*dt);
            
            // vec3 velocity = ph::toGlmVector(body->getLinearVelocity());
            // vec3 delta = targetVelocity - velocity;
            // body->applyLocalForceAtCenterOfMass(ph::toRp3dVector(delta) * body->getMass() * acceleration);
            if(clickInput) {
                ph::RaycastCallback callback;
                world->raycast(getEyePosition(),getEyeDirection(),10,&callback);
                if(callback.success) {
                    ActorUserData* data = static_cast<ActorUserData*>(callback.body->getUserData());
                    Construction* construction = dynamic_cast<Construction*>(data->actor);
                    if(construction != nullptr) {
                        vec3 placePointWorld = callback.worldPoint + callback.worldNormal * 0.5f;
                        vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                        ivec3 placePointLocalInt = glm::round(placePointLocal);
                        construction->setBlock(placePointLocalInt,1);
                    }
                }
                // if(callback.success) {
                //     ActorUserData* data = static_cast<ActorUserData*>(callback.body->getUserData());
                //     Construction* construction = dynamic_cast<Construction*>(data->actor);

                //     if(construction != nullptr) {
                //         vec3 placePointWorld = callback.worldPoint + callback.worldNormal * 0.5f;
                //         vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                //         ivec3 placePointLocalInt = glm::round(placePointWorld);
                //         construction->setBlock(placePointLocalInt,1);
                //         std::cout << "Hit point on construction! : " << StringHelper::toString(callback.worldPoint) << std::endl;
                //     } else {
                //         std::cout << "Hit point : " << StringHelper::toString(callback.worldPoint) << std::endl;
                //     }
                // }
            }

            RigidbodyActor::step(dt,world);
            //velocity.x = moveSpeed;
            //std::cout << std::format("<{},{},{}>",position.x,position.y,position.z) << std::endl;

            clickInput = false;

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

            if(input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1)) {
                clickInput = true;
            }

            moveMouse(input.getMouseDelta() * 0.01f);
        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,vec3(0,1,0)) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void firstPersonCamera(Camera& camera) {
            camera.position = getEyePosition();
            camera.rotation = getEyeRotation();
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