#pragma once

#include "GLFW/glfw3.h"

#include "actor/actor.hpp"
#include <memory>

using glm::vec3, glm::quat,std::unique_ptr;

#define WORLD
class World {

    vector<unique_ptr<Actor>> actors;
    vector<unique_ptr<Actor>> spawnedActors; //for when we spawn in the step
    vec3 constantGravity = vec3(0,-15,0);

    Camera camera;

    float sinceLastStep;

    struct WorldRaycastHit {
        Actor* actor;
        RaycastHit hit;

        WorldRaycastHit(Actor* actor,RaycastHit hit) : actor(actor), hit(hit) {

        }
    };
   
    // class DebugCallback : public rp3d::RaycastCallback {
    
    //     public:
    
    //     virtual rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) {
    
    //         // Display the world hit point coordinates
    //         Debug::drawPoint(ph::toGlmVector(info.worldPoint));

    //         std::cout << "Hit point : " << StringHelper::toString(ph::toGlmVector(info.worldPoint)) << std::endl;
    
    //         return info.hitFraction;
    //     }
    // };
        

    public:

        float stepDt = 0.02;

        float stepProcessMs;
        float renderProcessMs;

        bool iteratingActors = false; //so we dont resize the actor vector when iterating over it

        World() {
        }

        template<typename T>
        T* spawn(unique_ptr<T> spawned) {
            T* rawSpawned = spawned.get();
            if(iteratingActors) {
                spawnedActors.push_back(std::move(spawned));
            } else {
                actors.push_back(std::move(spawned));
            }
            return rawSpawned;
        }

        void destroy(Actor* actor) {

        }

        vec3 getGravityVector(vec3 position) {
            return constantGravity;
        }
        
        // do rendering, step and everything else
        void frame(Vulkan* vulkan,float dt) {
            
            float clock = glfwGetTime();
            addRenderables(vulkan,dt);
            renderProcessMs = ((float)glfwGetTime() - clock) * 1000;

            sinceLastStep += dt;
            if(sinceLastStep > 0.1f) {
                sinceLastStep = 0.1f; //we dont want to get caught in an infinite loop! if the physics takes too long the game will just slow down
            }
            //max 5 steps before we render again (dont wanna get caught in an infinite loop!)
            while (sinceLastStep > stepDt)
            {
                clock = glfwGetTime();
                sinceLastStep -= stepDt;
                step(stepDt);
                stepProcessMs = ((float)glfwGetTime() - clock) * 1000;

            }
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            for (auto& actor : actors)
            {
                actor->addRenderables(vulkan,dt);
            }
            
        }

        void step(float dt) {
            iteratingActors = true;
            for (auto& actor : actors)
            {
                actor->step(this,dt);
            }
            iteratingActors = false;
            for(auto& actor : spawnedActors) {
                actors.push_back(std::move(actor));
            }
            spawnedActors.clear();
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            //if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }

        std::optional<WorldRaycastHit> raycast(Ray ray,float dist) {
            std::optional<WorldRaycastHit> result = std::nullopt;
            for (auto& actor : actors)
            {
                auto hitopt = actor->raycast(ray,dist);
                if(hitopt) {
                    auto hit = hitopt.value();
                    if(hit.distance <= dist) {
                        if(result) {
                            result.value().hit = hit;
                            result.value().actor = actor.get();
                        } else {
                            result = WorldRaycastHit(actor.get(),hit);
                        }
                        dist = hit.distance;
                    }
                }
            }
            return result;
        }

        void collideBasic(Actor* actor,float radius) {
            iteratingActors = true;
            for (auto& colliderActor : actors)
            {
                if(actor != colliderActor.get()) {
                    colliderActor->collideBasic(actor,radius);
                }
                
            }
            iteratingActors = false;
        }

        Camera& getCamera() {
            return camera;
        }

};