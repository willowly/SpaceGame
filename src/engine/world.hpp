#pragma once

#include "GLFW/glfw3.h"

#include "actor/actor.hpp"
#include <memory>
// #include <tracy/Tracy.hpp>
#include "physics/physics-body.hpp"

#include <Jolt/Jolt.h>

#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

#include "physics/jolt-layers.hpp"

using glm::vec3, glm::quat, std::unique_ptr;

#define WORLD
class World {

    vector<unique_ptr<Actor>> actors;
    vector<unique_ptr<Actor>> spawnedActors; //for when we spawn in the step
    vec3 constantGravity = vec3(0,-15,0);

    Camera camera;

    float sinceLastStep;

    //physics stuff
    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    JPH::TempAllocatorImpl* temp_allocator;
    JPH::JobSystemThreadPool* jobSystem;



    struct WorldRaycastHit {
        Actor* actor;
        RaycastHit hit;

        WorldRaycastHit(Actor* actor,RaycastHit hit) : actor(actor), hit(hit) {

        }
    };



        void setupPhysics() {

            JPH::RegisterDefaultAllocator(); //we use the default allocator for jolt
            JPH::Factory::sInstance = new JPH::Factory(); //set the factory singleton
            JPH::RegisterTypes(); //idfk

            temp_allocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
            jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
            const unsigned int cMaxBodies = 1024;

            // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
            const unsigned int cNumBodyMutexes = 0;
            // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
            const unsigned int cMaxBodyPairs = 1024;
            // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
            const unsigned int cMaxContactConstraints = 1024;

            physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);

            JPH::BodyInterface& body_interface = physics_system.GetBodyInterfaceNoLock();

            // create floor

            JPH::BodyCreationSettings floor_settings(new JPH::BoxShape(JPH::Vec3(100.0f, 1.0f, 100.0f)), JPH::Vec3(0.0, -1.0, 0.0), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

            JPH::Body *floor = body_interface.CreateBody(floor_settings);

            body_interface.AddBody(floor->GetID(), JPH::EActivation::DontActivate);


        }

                
        

    public:

        // for now
        JPH::PhysicsSystem physics_system;

        // not sure how to solve this one :)
        Material constructionMaterial = Material::none;

        float stepDt = 0.02;

        float stepProcessMs;
        float renderProcessMs;

        std::vector<PhysicsBody*> physicsBodies;
        bool pausePhysics;
        bool stepPhysics; //trigger

        int iteratingActors = 0; //so we dont resize the actor vector when iterating over it. int so that we can have nested iterations



        World() {
            setupPhysics();
        }

        vec3 testPointA;
        vec3 testPointB;
        vec3 testPointC;

        template<typename T>
        T* spawn(unique_ptr<T> spawned) {
            T* rawSpawned = spawned.get();
            if(iteratingActors > 0) {
                spawnedActors.push_back(std::move(spawned));
            } else {
                actors.push_back(std::move(spawned));
            }

            rawSpawned->spawn(this); // do actor specific spawning code

            return rawSpawned;
        }

        vec3 getGravityVector(vec3 position) {
            return constantGravity;
        }
        
        // do rendering, step and everything else
        void frame(Vulkan* vulkan,float dt) {
            
            //ZoneScoped;
            
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
            //ZoneScoped;
            for (auto& actor : actors)
            {
                actor->addRenderables(vulkan,dt);
            }
            
        }

        void step(float dt) {
            //ZoneScoped;
            
            iteratingActors++;
            for (auto& actor : actors)
            {
                actor->step(this,dt);
            }
            // remove the ones marked as destroyed
            for (int i = actors.size() - 1; i >= 0; i--)
            {
                auto& actor = actors[i];
                if(actor->destroyed) {
                    actors.erase(actors.begin()+i);
                }
            }
            iteratingActors--;

            physics_system.SetGravity(JPH::Vec3(0.0,-10.0,0.0));

            physics_system.Update(dt,1,temp_allocator,jobSystem);

            for(auto& actor : spawnedActors) {
                actors.push_back(std::move(actor));
            }
            spawnedActors.clear();
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            //if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }

        std::optional<WorldRaycastHit> raycast(Ray ray,float dist) {
            //ZoneScoped;
            std::optional<WorldRaycastHit> result = std::nullopt;
            iteratingActors++;
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
            iteratingActors--;
            return result;
        }

        void collideBasic(Actor* actor,float height,float radius) {
            //ZoneScoped;
            iteratingActors++;
            for (auto& colliderActor : actors)
            {
                if(actor != colliderActor.get()) {
                    colliderActor->collideBasic(actor,height,radius);
                }
                
            }
            iteratingActors--;
        }

        void addPhysicsBody(PhysicsBody* body) {
            physicsBodies.push_back(body);
        }

        void removePhysicsBody(PhysicsBody* body) {
            std::erase(physicsBodies,body);
        }

        Camera& getCamera() {
            return camera;
        }

};