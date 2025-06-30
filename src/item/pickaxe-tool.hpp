#include "tool.hpp"


class PickaxeTool : public Tool {

    public:
        void use(World* world,vec3 position,vec3 direction) {
            ph::RaycastCallback callback;
            world->raycast(position,direction,10,&callback);
            if(callback.success) {
                ActorUserData* data = static_cast<ActorUserData*>(callback.body->getUserData());
                Construction* construction = dynamic_cast<Construction*>(data->actor);
                if(construction != nullptr) {
                    vec3 placePointWorld = callback.worldPoint - callback.worldNormal * 0.5f;
                    vec3 placePointLocal = construction->inverseTransformPoint(placePointWorld);
                    ivec3 placePointLocalInt = glm::round(placePointLocal);
                    construction->setBlock(placePointLocalInt,nullptr);
                }
            }
        }


};