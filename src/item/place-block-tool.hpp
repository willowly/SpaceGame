#include "tool.hpp"


class PlaceBlockTool: public Tool {

    
    public:
        Block* block;

        PlaceBlockTool() {
            
        }

        PlaceBlockTool(Block* block) : Tool(block->model,block->material), block(block) {

        }

        quat placeAnimationRotation = glm::quat(vec3(glm::radians(-50.0f),0,0));
        float placeAnimationTime = 0.2;

        float placeAnimationTimer = 0;
    
        void place(World* world,Ray ray) {

            auto worldHitOpt = world->raycast(ray,10);
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = worldHit.hit.point + worldHit.hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    auto facing = Construction::getFacingFromVector(construction->inverseTransformDirection(worldHit.hit.normal));
                    construction->setBlock(placePointLocalInt,block,facing);
                }
                // Terrain* terrain = dynamic_cast<Terrain*>(worldHit.actor);
                // if(terrain != nullptr) {
                //     auto construction = world->spawn<Construction>(worldHit.hit.point,glm::quatLookAt(worldHit.hit.normal,vec3(0,1,0)));
                //     construction->setBlock(ivec3(0),block,BlockFacing::FORWARD);
                // }
            } else {
                auto construction = world->spawn<Construction>(ray.origin + ray.direction*10.0f,glm::quatLookAt(ray.direction,vec3(0,1,0)));
                construction->setBlock(ivec3(0),block,BlockFacing::FORWARD);
            }
            placeAnimationTimer = placeAnimationTime;
        }

        virtual std::pair<quat,vec3> animate(float dt) {

            if(placeAnimationTimer > 0) {
                placeAnimationTimer -= dt;
                return std::pair<quat,vec3>(glm::slerp(glm::identity<quat>(),placeAnimationRotation,placeAnimationTimer),vec3());
            }

            return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
        }


        virtual void step(World* world,ToolUser* user,float dt) {
            if(clickInput) {
                clickInput = false;
                place(world,user->getLookRay());
            }
        }


};