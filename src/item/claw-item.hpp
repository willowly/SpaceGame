#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/anim.hpp"
#include "helper/math-helper.hpp"
#include "math.h"
#pragma once


class ClawTool : public Tool {

    public:

        ClawTool() {

        }


        enum State {
            NEUTRAL,
            ANTICIPATION,
            COOLDOWN
        };


        string getTypeName() override {
            return "claw_tool";
        }

        int range = 30;
        float force = 3;
        float moveForce = 0.1f;
        float velocityThreshold = 5.0f; //below this amount and the velocity wont
        float angularDamping = 1.0f;
        float scrollSpeed = 0.5f;

        const int DIST_VAR = 0;

        virtual void equip(Character& user) {
            Tool::equip(user);
        }

        virtual ItemDisplayData getItemDisplay(ItemStack& stack) {
            return 0;
        }

        virtual void unequip(Character& user) {
            user.heldItemData.attachedActor = nullptr;
        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

            if(user.heldItemData.clickInput) {
                user.heldItemData.clickInput = false;
                if(user.heldItemData.attachedActor != nullptr) {
                    auto actor = user.heldItemData.attachedActor;
                    if(actor->hasVelocity()) {
                        auto velocity = actor->getVelocity();
                        if(glm::length(velocity) < velocityThreshold) {
                            actor->setVelocity(vec3(0.0f));
                        }
                    }
                    user.heldItemData.attachedActor = nullptr;
                } else {
                    Ray ray = user.getLookRay();
                    auto hitOpt = world->raycast(ray,range,LayerMask::excludes({Layers::PLAYER}));
                    if(hitOpt) {

                        auto hit = hitOpt.value();
                        stack.storage.setFloat(DIST_VAR,glm::distance(hit.actor->getPosition(),user.getEyePosition()));

                        user.heldItemData.attachedActor = hit.actor;
                    }
                }
            }
            if(user.heldItemData.attachedActor != nullptr) {
                auto actor = user.heldItemData.attachedActor;
                auto distance = stack.storage.getFloat(DIST_VAR);
                distance += user.heldItemData.scrollDelta * scrollSpeed;
                if(distance < 2) {
                    distance = 2;
                }
                if(distance > range) {
                    distance = range;
                }
                if(user.heldItemData.altClickHold) {
                    auto rotation = actor->getRotation();
                    rotation *= quat(user.transformDirection(vec3(0,user.heldItemData.mouseDelta.x * dt,0)));
                    rotation *= quat(user.transformDirection(vec3(user.heldItemData.mouseDelta.y * dt,0,0)));
                    actor->setRotation(rotation);
                }
                stack.storage.setFloat(DIST_VAR,distance);
                auto position = actor->getPosition();
                auto target = user.getLookRay().origin + user.getLookRay().direction * distance;
                if(actor->hasAngularVelocity()) {
                    auto angularVelocity = actor->getAngularVelocity();
                    angularVelocity = MathHelper::moveTowards(angularVelocity,vec3(0),angularDamping * dt);
                    actor->setAngularVelocity(angularVelocity);
                }
                if(actor->hasVelocity()) {
                    auto velocity = (target - position) * force;
                    actor->setVelocity(velocity);
                } else {
                    position = MathHelper::lerp(position,target,force * dt);
                    actor->setPosition(position);
                }
            }
        
        }


};