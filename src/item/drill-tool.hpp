#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/anim.hpp"
#include "math.h"
#pragma once


class DrillTool : public Tool {

    public:

        DrillTool() {

        }


        enum State {
            NEUTRAL,
            ANTICIPATION,
            COOLDOWN
        };

        float mineRadius = 0.75;
        float mineAmount = 0.5;

        float reach = 5;

        int durability = 10;

        string getTypeName() override {
            return "drill_tool";
        }

        const int DAMAGE_VAR = 0;

        void drill(World* world,Character& user,ItemStack& stack,Ray ray,float dt) {
            
           
        }

        virtual void equip(Character& user) {
            Tool::equip(user);
        }

        virtual ItemDisplayData getItemDisplay(ItemStack& stack) {
            float damage = static_cast<float>(stack.storage.getInt(DAMAGE_VAR,0));
            return ItemDisplayData(1-(damage/durability));
        }



        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

            if(!user.heldItemData.clickHold) {
                return;
            }
            
            float damage = stack.storage.getFloat(DAMAGE_VAR,0);
            
            if(damage >= durability) {
                return;
            }
            auto hitOpt = world->raycast(user.getLookRay(),reach,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));
            std::optional<ItemStack> resultStack; //constructions :shrug:
            if(hitOpt) {
                auto hit = hitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(hit.actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = hit.point - hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    auto blockPair = construction->getBlock(placePointLocalInt);
                    construction->breakBlock(world,placePointLocalInt);
                    //user.shake.startShake();
                    damage += dt;
                }
                Terrain* terrain = dynamic_cast<Terrain*>(hit.actor);
                if(terrain != nullptr) {
                    terrain->terraformSphere(world,hit.point,mineRadius,-mineAmount * dt);

                    //user.shake.startShake();
    
                    damage += dt;
                }
            }
            //std::cout << "damage: " << damage << std::endl;
            stack.storage.setFloat(DAMAGE_VAR,damage);
        
        }


};