#pragma once
#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/block-helper.hpp"


class HammerTool : public Tool {

    private:

        

    public:

        enum class ActionEvent {
            Hammer = 0,
        };

        string getTypeName() override {
            return "hammer_tool";
        }

        HammerTool() {
            
        }
    
        bool hammer(World* world,Character& user) {

            Ray ray = user.getLookRay();
            auto hitOpt = world->raycast(ray,10,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));
            if(hitOpt) {
                auto hit = hitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(hit.actor);
                if(construction != nullptr) {
                    
                    vec3 placePointWorld = hit.point - hit.normal * 0.16f; //about 1/6, half of a 3rd of the cube.
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);

                    auto blockPalette = construction->getBlock(placePointLocalInt);
                    auto blockData = construction->getBlockData(placePointLocalInt);

                    if(blockPalette.block != nullptr) {
                        blockPalette.block->onHammer(construction,placePointLocal,placePointLocalInt,blockPalette.storage);
                    }
                    return true;
                }
            }
            return false;
        }


        void receiveActionEvent(World* world,Character& user,ItemStack& stack,int actionEvent) override {
            switch((ActionEvent)actionEvent) {
                case ActionEvent::Hammer:
                    if(hammer(world,user)) {
                        
                        user.refreshTool(); //in case we run out
                    }
                    break;
            }
        }


        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {
            if(user.heldItemData.clickInput) {
                user.heldItemData.clickInput = false;
                if(hammer(world,user)) {
                    user.heldItemData.setAction(0);
                    user.refreshTool(); //in case we run out
                }
            }
        }

        virtual void stepClient(World* world,Character& user,ItemStack& stack,float dt) {
            if(user.heldItemData.clickInput) {
                user.heldItemData.clickInput = false;
                sendActionEvent(user,(int)ActionEvent::Hammer);
            }
        }


};