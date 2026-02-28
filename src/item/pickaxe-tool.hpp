#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/anim.hpp"
#include "math.h"
#pragma once


class PickaxeTool : public Tool {

    public:

        PickaxeTool() {

        }

        PickaxeTool(Mesh<Vertex>* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation) : Tool(heldModel,heldModelMaterial,modelOffset,modelRotation) {
            
        }

        enum State {
            NEUTRAL,
            ANTICIPATION,
            COOLDOWN
        };

        quat anticipationRotation = glm::quat(vec3(0,0,glm::radians(50.0f)));
        float anticipationTime = 0.3f;
        quat cooldownRotation = glm::quat(vec3(0,0,glm::radians(-50.0f)));
        float cooldownTime = 0.5;

        float mineRadius = 0.75;
        float mineAmount = 0.5;

        float reach = 5;

        int durability = 10;

        const int DAMAGE_VAR = 0;

        ParticleEffectActor* testEffect;

        void pickaxe(World* world,Character& user,ItemStack& stack,Ray ray) {
            
            int damage = stack.storage.getInt(DAMAGE_VAR,0);
            auto worldHitOpt = world->raycast(ray,reach);
            TerraformResults results; //terrain
            std::optional<ItemStack> resultStack; //constructions :shrug:
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    auto blockPair = construction->getBlock(placePointLocalInt);
                    if(blockPair.first != nullptr) {
                        resultStack = blockPair.first->getDrop(blockPair.second);
                        // give at the end to avoid errors
                    }
                    construction->breakBlock(world,placePointLocalInt);
                    user.shake.startShake();
                    damage++;
                }
                Terrain* terrain = dynamic_cast<Terrain*>(worldHit.actor);
                if(terrain != nullptr) {
                    results = terrain->terraformSphere(worldHit.hit.point,mineRadius,-mineAmount);
                    // give at the end to avoid errors
                    world->spawn(ParticleEffectActor::makeInstance(testEffect,worldHit.hit.point)); //hmm

                    user.shake.startShake();
    
                    damage++;
                }
            }
            //std::cout << "damage: " << damage << std::endl;
            stack.storage.setInt(DAMAGE_VAR,damage);
            if(damage >= durability) {
                stack.clear();
            }
            
            // good enough for now make sure these happen after, since they can invalidate stack references
            if(resultStack) {
                auto stack = resultStack.value();
                user.inventory.give(stack);
            }

            for(auto& stack : results.items) {
                user.inventory.give(stack);
            }
        }

        virtual void equip(Character& user) {
            Tool::equip(user);
        }

        virtual ItemDisplayData getItemDisplay(ItemStack& stack) {
            float damage = static_cast<float>(stack.storage.getInt(DAMAGE_VAR,0));
            return ItemDisplayData(1-(damage/durability));
        }


        

        virtual std::pair<quat,vec3> animate(Character& user,float dt) {
            float animationTimer = user.heldItemData.animationTimer;
            auto animation = std::pair<quat,vec3>(glm::identity<quat>(),vec3());
            float normTime;
            float easedTime;
            switch ((State)user.heldItemData.action) {
                case State::NEUTRAL:
                    break;
                case State::ANTICIPATION:

                    normTime = animationTimer/anticipationTime;
                    if(normTime > 0.9f) {
                        normTime = (normTime - 0.9f) / 0.1f;
                        normTime = fmin(fmax(normTime,0.0f),1.0f);
                        easedTime = Anim::easeOutCubic(normTime);
                        animation.first = glm::slerp(anticipationRotation,cooldownRotation,easedTime);
                    } else {
                        normTime = (normTime / 0.9f);
                        normTime = fmin(fmax(normTime,0.0f),1.0f);
                        easedTime = Anim::easeInSine(normTime);
                        animation.first = glm::slerp(glm::identity<quat>(),anticipationRotation,easedTime);
                    }
                    
                    break;
                case State::COOLDOWN:
                    normTime = animationTimer/cooldownTime;
                    easedTime = Anim::easeOutSine(normTime);
                    animation.first = glm::slerp(cooldownRotation,glm::identity<quat>(),easedTime);
                    break;
            }
            return animation;
            
        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {
            switch ((State)user.heldItemData.action) {
                case State::NEUTRAL:

                    if(clickHold) {
                        user.heldItemData.setAction(State::ANTICIPATION);
                    }
                    break;
                case State::ANTICIPATION:
                    if(user.heldItemData.actionTimer > anticipationTime) {
                        user.heldItemData.setAction(State::COOLDOWN);
                        pickaxe(world,user,stack,user.getLookRay());
                    }
                    break;
                case State::COOLDOWN:
                    if(user.heldItemData.actionTimer > cooldownTime) {
                        user.heldItemData.setAction(State::NEUTRAL);
                    }
                    break;
            }
        }


};