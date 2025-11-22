#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/anim.hpp"
#include "math.h"


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
        float anticipationTime = 0.3;
        quat cooldownRotation = glm::quat(vec3(0,0,glm::radians(-50.0f)));
        float cooldownTime = 0.5;

        float mineRadius = 0.75;
        float mineAmount = 0.5;

        float reach = 5;

        void pickaxe(World* world,Character& user,Ray ray) {
            
            auto worldHitOpt = world->raycast(ray,reach);
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    auto blockPair = construction->getBlock(placePointLocalInt);
                    if(blockPair.first != nullptr) {
                        auto stackOpt = blockPair.first->getDrop(blockPair.second);
                        if(stackOpt) {
                            auto stack = stackOpt.value();
                            user.inventory.give(stack);
                        }
                    }
                    construction->setBlock(placePointLocalInt,nullptr,BlockFacing::FORWARD);
                }
                Terrain* terrain = dynamic_cast<Terrain*>(worldHit.actor);
                if(terrain != nullptr) {
                    auto results = terrain->terraformSphere(worldHit.hit.point,mineRadius,-mineAmount);
                    terrain->generateMesh();
                    
                    if((results.stone) > 0) {
                        user.inventory.give(terrain->item1,results.stone);
                    }
                    if((results.ore) > 0) {
                        user.inventory.give(terrain->item2,results.ore);
                    }
                }
            }
        }

        virtual void equip(Character& user) {
            Tool::equip(user);
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
                        normTime = fmin(fmax(normTime,0),1);
                        easedTime = Anim::easeOutCubic(normTime);
                        animation.first = glm::slerp(anticipationRotation,cooldownRotation,easedTime);
                    } else {
                        normTime = (normTime / 0.9f);
                        normTime = fmin(fmax(normTime,0),1);
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
                        pickaxe(world,user,user.getLookRay());
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