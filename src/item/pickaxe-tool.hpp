#include "tool.hpp"
#include "actor/terrain.hpp"


class PickaxeTool : public Tool {

    public:

        PickaxeTool() {

        }

        PickaxeTool(Model* heldModel,Material* heldModelMaterial,vec3 modelOffset,quat modelRotation) : Tool(heldModel,heldModelMaterial,modelOffset,modelRotation) {
            
        }

        float mineRadius = 0.75;
        float mineAmount = 0.5;

        void pickaxe(World* world,Ray ray) {
            
            auto worldHitOpt = world->raycast(ray,10);
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    construction->setBlock(placePointLocalInt,nullptr,BlockFacing::FORWARD);
                }
                Terrain* terrain = dynamic_cast<Terrain*>(worldHit.actor);
                if(terrain != nullptr) {
                    terrain->terraformSphere(worldHit.hit.point,mineRadius,-mineAmount);
                    terrain->generateMesh();
                }
            }
        }

        virtual void step(World* world,ToolUser* user,float dt) {
            if(clickInput) {
                clickInput = false;
                pickaxe(world,user->getLookRay());
            }
        }


};