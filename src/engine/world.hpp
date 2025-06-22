#pragma once
#include "actor/actor.hpp"
#include <memory>
#include "actor/physics-actor.hpp"
#include "helper/collision-helper.hpp"

#include <reactphysics3d/reactphysics3d.h>

using glm::vec3, glm::quat,std::unique_ptr;

class World {

    vector<unique_ptr<Actor>> actors;
    vector<PhysicsActor*> physicsActors; //they need to be handled differently
    vec3 constantGravity = vec3(0,-15,0);

    rp3d::PhysicsWorld* physicsWorld;
    rp3d::PhysicsCommon* physicsCommon;


    public:
        template <typename T>
        T* spawn(T* prototype,vec3 position,quat rotation) {
            Actor* actorPointer = new T(*prototype);
            actors.push_back(std::unique_ptr<Actor>(actorPointer));
            actors[actors.size()-1]->position = position;
            actors[actors.size()-1]->rotation = rotation;
            if constexpr(std::is_same_v<T,PhysicsActor>) {
                PhysicsActor* physicsActor = dynamic_cast<PhysicsActor*>(actorPointer);
                physicsActor->addToWorld(physicsWorld,physicsCommon);
                physicsActors.push_back(physicsActor);
            }
            return dynamic_cast<T*>(actorPointer);
        }


        vec3 getGravityVector(vec3 position) {
            return constantGravity;
        }

        void render(Camera& camera) {
            for (auto& actor : actors)
            {
                actor->render(camera);
            }
            
        }

        void step(float dt) {
            for (auto& actor : actors)
            {
                if(actor->playerStep()) continue;
                //applyGravityIfEnabled(actor.get(),dt);
                actor->step(dt);
            }
            //do physics
            for(auto physicsActor : physicsActors) {
                physicsActor->updatePhysicsRepresentation();
            }
            physicsStep(dt);
            for(auto physicsActor : physicsActors) {
                physicsActor->updateFromPhysicsRepresentation();
            }
        }

        void physicsStep(float dt) {
            physicsWorld->update(dt);
        }

        void playerStep(float dt) {
            for (auto& actor : actors)
            {
                if(!actor->playerStep()) continue;
                applyGravityIfEnabled(actor.get(),dt);
                actor->step(dt);
            }
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }



        rp3d::PhysicsWorld* getPhysicsWorld() {
            return physicsWorld;
        }

        void setPhysicsWorld(rp3d::PhysicsWorld* physicsWorld) {
            this->physicsWorld = physicsWorld;
        }

        rp3d::PhysicsCommon* getPhysicsCommon() {
            return physicsCommon;
        }

        void setPhysicsCommon(rp3d::PhysicsCommon* physicsCommon) {
            this->physicsCommon = physicsCommon;
        }

};