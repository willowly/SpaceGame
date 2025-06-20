#pragma once
#include "actor/actor.hpp"
#include <memory>

using glm::vec3, glm::quat,std::unique_ptr;

class World {

    vector<unique_ptr<Actor>> actors;
    vec3 constantGravity = vec3(0,-15,0);


    public:
        template <typename T>
        T* spawn(T* prototype,vec3 position,quat rotation) {
            Actor* actorPointer = new T(*prototype);
            actors.push_back(std::unique_ptr<Actor>(actorPointer));
            actors[actors.size()-1]->position = position;
            actors[actors.size()-1]->rotation = rotation;
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
                if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
                actor->step(dt);
            }
        }

};