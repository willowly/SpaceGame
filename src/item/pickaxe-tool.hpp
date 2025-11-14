#include "tool.hpp"
//#include "actor/terrain.hpp"


class PickaxeTool : public Tool {

    public:

        PickaxeTool() {

        }

        PickaxeTool(Mesh* heldModel,Material heldModelMaterial,vec3 modelOffset,quat modelRotation) : Tool(heldModel,heldModelMaterial,modelOffset,modelRotation) {
            
        }

        enum State {
            NEUTRAL,
            ANTICIPATION,
            COOLDOWN
        };

        State state;

        quat anticipationRotation = glm::quat(vec3(0,0,glm::radians(50.0f)));
        float anticipationTime = 0.3;
        quat cooldownRotation = glm::quat(vec3(0,0,glm::radians(-50.0f)));
        float cooldownTime = 0.5;

        float stateTimer = 0;
        float animationTimer = 0;

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
                // Terrain* terrain = dynamic_cast<Terrain*>(worldHit.actor);
                // if(terrain != nullptr) {
                //     terrain->terraformSphere(worldHit.hit.point,mineRadius,-mineAmount);
                //     terrain->generateMesh();
                // }
            }
        }

        virtual void equip(ToolUser* user) {
            state = State::NEUTRAL;
            stateTimer = 0;
            Tool::equip(user);
        }

        void setState(State newState) {
            state = newState;
            stateTimer = 0;
            animationTimer = 0;
        }

        virtual std::pair<quat,vec3> animate(float dt) {
            animationTimer += dt;
            switch (state) {
                case State::NEUTRAL:
                    return std::pair<quat,vec3>(glm::identity<quat>(),vec3());
                case State::ANTICIPATION:
                    return std::pair<quat,vec3>(glm::slerp(glm::identity<quat>(),anticipationRotation,animationTimer/anticipationTime),vec3());
                case State::COOLDOWN:
                    return std::pair<quat,vec3>(glm::slerp(cooldownRotation,glm::identity<quat>(),animationTimer/cooldownTime),vec3());
            }
            
        }

        virtual void step(World* world,ToolUser* user,float dt) {
            switch (state) {
                case State::NEUTRAL:

                    if(clickHold) {
                        setState(State::ANTICIPATION);
                    }
                    break;
                case State::ANTICIPATION:
                    if(stateTimer > anticipationTime) {
                        setState(State::COOLDOWN);
                        pickaxe(world,user->getLookRay());
                    }
                    break;
                case State::COOLDOWN:
                    if(stateTimer > cooldownTime) {
                        setState(State::NEUTRAL);
                    }
                    break;
            }

            stateTimer += dt;
        }


};