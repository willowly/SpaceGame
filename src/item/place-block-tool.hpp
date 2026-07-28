#pragma once
#include "tool.hpp"
#include "actor/terrain.hpp"
#include "helper/block-helper.hpp"


class PlaceBlockTool: public Tool {

    private:

        enum class PlacementBlockedResult {
            Clear,
            Blocked,
            Terrain,
        };

        PlacementBlockedResult isPlacementBlocked(World* world,vec3 pointWorld,quat rotation) {
            auto overlapResult = world->overlapBox(pointWorld,vec3(0.9f),rotation);
            if(overlapResult) {
                auto actor = overlapResult.value();
                Terrain* terrain = dynamic_cast<Terrain*>(actor);
                if(terrain != nullptr) {
                    return PlacementBlockedResult::Terrain;
                }
                return PlacementBlockedResult::Blocked;
            }
            return PlacementBlockedResult::Clear;
        }

        struct TryPlaceResult {
            Construction* construction = nullptr;
            vec3 position = {};
            quat rotation = {};
            vec3 velocity = {};
            ivec3 localBlockPos = {};
            BlockPlaceInfo placeInfo;
        };

        std::optional<TryPlaceResult> tryPlace(World* world,Character& user) {


            TryPlaceResult result;
            Ray ray = user.getLookRay();
            auto hitOpt = world->raycast(ray,10,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));
            if(hitOpt) {
                auto hit = hitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(hit.actor);
                if(construction != nullptr) {

                    vec3 placePointWorld = hit.point + hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);

                    placePointWorld = construction->transformPoint(placePointLocalInt);

                    auto testResult = isPlacementBlocked(world,placePointWorld,construction->getRotation());
                    if(testResult == PlacementBlockedResult::Blocked) {
                        std::cout << "placement blocked" << std::endl;
                        // cannot place
                        return std::nullopt; 
                    }

                    if(testResult == PlacementBlockedResult::Terrain) {
                        std::cout << "placed attached" << std::endl;
                        result.placeInfo.attached = true;
                    } else {
                        result.velocity = user.getVelocity();
                    }
                    result.placeInfo.lookDir = construction->inverseTransformDirection(ray.direction);
                    result.placeInfo.normal = construction->inverseTransformDirection(hit.normal);
                    result.position = placePointWorld;
                    result.rotation = construction->getRotation();
                    result.localBlockPos = placePointLocalInt;
                    result.construction = construction;
                    return result;
                }
                Terrain* terrain = dynamic_cast<Terrain*>(hit.actor);
                if(terrain != nullptr) {
                    result.position = hit.point+glm::vec3(0,0.4,0);
                    result.rotation = user.getRotation();
                    return result;
                }
            } else {
                result.position = ray.origin + ray.direction*10.0f;
                result.rotation = glm::quatLookAt(ray.direction,vec3(0,1,0));
                return result;
            }
            return std::nullopt;
        }

        bool place(World* world,Character& user) {

            auto placeResultOpt = tryPlace(world,user);
            if(placeResultOpt) {
                auto placeResult = placeResultOpt.value();
                if(placeResult.construction == nullptr) {
                    auto construction = world->spawn(Construction::makeInstance(world->constructionMaterial,block,placeResult.position,placeResult.rotation,false));
                } else {
                    placeResult.construction->placeBlock(placeResult.localBlockPos,block,placeResult.placeInfo);
                }
                return true;
            }
            return false;
        }

    public:

        enum class ActionEvent {
            Place = 0,
        };
        Block* block = nullptr;

        string getTypeName() override {
            return "place_block_tool";
        }

        PlaceBlockTool() {
            
        }

        quat placeAnimationRotation = glm::quat(vec3(glm::radians(-50.0f),0,0));
        float placeAnimationTime = 0.2;

        void addRenderablesHeld(Vulkan* vulkan,Character& user,float dt,float interpolation) override {
            Tool::addRenderablesHeld(vulkan,user,dt,interpolation);

        }

        virtual std::pair<quat,vec3> animate(Character& user,float dt) {

            if(user.heldItemData.animationTimer < placeAnimationTime) {
                return std::pair<quat,vec3>(glm::slerp(glm::identity<quat>(),placeAnimationRotation,placeAnimationTime - user.heldItemData.animationTimer),vec3());
            }

            return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
        }

        void receiveActionEvent(World* world,Character& user,ItemStack& stack,int actionEvent) override {
            switch((ActionEvent)actionEvent) {
                case ActionEvent::Place:
                    if(place(world,user)) {
                        user.take(ItemStack(stack.item,1));
                        user.refreshTool(); //in case we run out
                    }
                    break;
            }
        }


        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {
            if(user.heldItemData.clickInput) {
                user.heldItemData.clickInput = false;
                if(place(world,user)) {
                    user.heldItemData.setAction(0);
                    stack.amount--;
                    user.refreshTool(); //in case we run out
                }
            }
        }

        virtual void stepClient(World* world,Character& user,ItemStack& stack,float dt) {
            if(user.heldItemData.clickInput) {
                user.heldItemData.clickInput = false;
                sendActionEvent(user,(int)ActionEvent::Place);
            }
        }


};