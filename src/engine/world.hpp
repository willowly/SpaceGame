#pragma once

#include "GLFW/glfw3.h"

#include "actor/actor.hpp"
#include <memory>
//#include <tracy/Tracy.hpp>
#include "physics/jolt-physics-body.hpp"

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
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

#include "physics/jolt-layers.hpp"
#include "physics/jolt-trace.hpp"
#include "physics/jolt-terrain-shape.hpp"
#include "physics/jolt-userdata.hpp"

#include "persistance/data-world.hpp"
#include "persistance/data-loader.hpp"

using glm::vec3, glm::quat, std::unique_ptr, std::shared_ptr;

#define WORLD
class World {

    struct ContactListener : public JPH::ContactListener {

        std::vector<Collision> collisions;
        std::mutex collisionsMutex;

        virtual void OnContactAdded([[maybe_unused]] const JPH::Body &inBody1, [[maybe_unused]] const JPH::Body &inBody2, [[maybe_unused]] const JPH::ContactManifold &inManifold, [[maybe_unused]] JPH::ContactSettings &ioSettings) {
        
            auto userDataRaw1 = inBody1.GetUserData();
            auto userDataRaw2 = inBody2.GetUserData();

            ActorUserData* userData1 = nullptr;
            Actor* actor1 = nullptr;
            if(userDataRaw1 != 0) {
                userData1 = ActorUserData::decode(userDataRaw1);
                actor1 = userData1->actor;
            }

            ActorUserData* userData2 = nullptr;
            Actor* actor2 = nullptr;
            if(userDataRaw2 != 0) {
                userData2 = ActorUserData::decode(userDataRaw2);
                actor2 = userData2->actor;
            }

            if(actor1 != nullptr && userData1->generateCollisionEvents) {
                std::scoped_lock lock(collisionsMutex);
                Collision collision{
                    .actor = actor1,
                    .body = inBody1.GetID(),
                    .otherActor = actor2,
                    .otherBody = inBody2.GetID(),
                    .inManifold = inManifold,
                };
                collisions.push_back(collision);
            }

            if(actor2 != nullptr && userData2->generateCollisionEvents) {
                std::scoped_lock lock(collisionsMutex);
                Collision collision{
                    .actor = actor2,
                    .body = inBody2.GetID(),
                    .otherActor = actor1,
                    .otherBody = inBody1.GetID(),
                    .inManifold = inManifold,
                };
                collisions.push_back(collision);
            }
        
        }
    };

    vector<shared_ptr<Actor>> actors;
    vector<shared_ptr<Actor>> spawnedActors; //for when we spawn in the step
    vec3 constantGravity = vec3(0,-15,0);

    Camera camera;

    ContactListener contactListener;

    float sinceLastStep = 0;

    //physics stuff
    BPLayerInterfaceImpl broad_phase_layer_interface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;

    JPH::TempAllocatorImpl* temp_allocator;
    JPH::JobSystemThreadPool* jobSystem;



    struct WorldRaycastHit {
        Actor* actor;
        RaycastHit hit;
        int component;

        WorldRaycastHit(Actor* actor,RaycastHit hit,int component = 0) : actor(actor), hit(hit), component(component) {

        }
    };



        void setupPhysics() {

            JPH::RegisterDefaultAllocator(); //we use the default allocator for jolt
            JPH::Factory::sInstance = new JPH::Factory(); //set the factory singleton
            JPH::RegisterTypes(); //idfk
            TerrainShape::sRegister();

            temp_allocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
            jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);
            //jobSystem = new JPH::JobSystemThreadPool(1, 1, 1);
            const JPH::uint cMaxBodies = 65536;

            JPH::Trace = Physics::TraceImpl;

            JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = Physics::AssertFailedImpl;)

            // This determines how many mutexes to allocate to protect rigid bodies from concurrent access. Set it to 0 for the default settings.
            const JPH::uint cNumBodyMutexes = 0;
            // Note: This value is low because this is a simple test. For a real project use something in the order of 65536.
            const JPH::uint cMaxBodyPairs = 65536;
            // Note: This value is low because this is a simple test. For a real project use something in the order of 10240.
            const JPH::uint cMaxContactConstraints = 10240;

            physics_system.SetContactListener(&contactListener);

            physics_system.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broad_phase_layer_interface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);


        }

                
        

    public:

        
        JPH::PhysicsSystem physics_system;

        // Global resources. Idk what to do about this tbh
        Material constructionMaterial = Material::none;

        float stepDt = 1.0f/60.0f;

        float stepProcessMs;
        float renderProcessMs;

        
        bool pausePhysics;
        bool stepPhysics; //trigger

        int iteratingActors = 0; //so we dont resize the actor vector when iterating over it. int so that we can have nested iterations



        World() {
            setupPhysics();
        }

        template<typename T>
        shared_ptr<T> spawn(unique_ptr<T> spawned) {

            shared_ptr<T> shared = std::move(spawned);
            if(iteratingActors > 0) {
                spawnedActors.push_back(shared);
            } else {
                actors.push_back(shared);
            }

            shared->spawn(this); // do actor specific spawning code

            return shared;
        }

        vec3 getGravityVector(vec3 position) {
            return constantGravity;
        }

        //probably should remove this, but its for getting the player. Ill think of a better way to handle this I guess
        template <typename T>
        std::shared_ptr<T> getActorOfType() {
            
            iteratingActors++;
            for (auto& actor : actors)
            {
                std::shared_ptr<T> typed_actor = std::dynamic_pointer_cast<T>(actor);
                if(typed_actor != nullptr) {
                    iteratingActors--;
                    return typed_actor;
                }
                
            }
            iteratingActors--;

            return nullptr;

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

        void physicsStep(float dt) {
            
            //ZoneScoped;

            {
                //ZoneScopedN("prePhysics")
                iteratingActors++;
                for (auto& actor : actors)
                {
                    actor->prePhysics(this);
                }
                iteratingActors--;
            }

            physics_system.SetGravity(JPH::Vec3(0.0,-10.0,0.0));

            physics_system.Update(dt,1,temp_allocator,jobSystem);

            {
                //ZoneScopedN("postPhysics")
                iteratingActors++;
                for (auto& actor : actors)
                {
                    actor->postPhysics(this);
                }
                iteratingActors--;
            }

            for (auto& collision : contactListener.collisions)
            {
                collision.actor->collisionStart(this,collision);
            }
            contactListener.collisions.clear();
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

            // this needs to happen first, because newly spawned actors need to have prePhysics() and postPhysics() called on them as well, since they are already in the physic system
            for(auto& actor : spawnedActors) {
                actors.push_back(std::move(actor));
            }
            spawnedActors.clear();

            physicsStep(dt);

            
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            //if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }

        std::vector<Actor*> overlapSphere(vec3 pos,float radius) {
            // JPH::ShapeCast shapecast();
            // JPH::SphereShape shape(radius);
            // physics_system.GetNarrowPhaseQuery().CollideShape(&shape,JPH::Vec3(1.0f,1.0f,1.0f),)

            std::vector<Actor*> results;

            iteratingActors++;
            for (auto& actor : actors)
            {
                if(glm::length2(actor->getPosition() - pos) < radius*radius) {
                    results.push_back(actor.get());
                }
            }
            iteratingActors--;

            return results;
            
        }

        std::optional<WorldRaycastHit> raycast(Ray ray,float dist) {
            //ZoneScoped;
            //std::optional<WorldRaycastHit> result = std::nullopt; 
            ray.direction = glm::normalize(ray.direction);

            
            JPH::RayCast raycast(Physics::toJoltVec(ray.origin),Physics::toJoltVec(ray.direction * dist));
            JPH::RayCastResult result;

            BroadPhaseLayerFilter broadFilter(BroadPhaseLayerTable{true,true,false}); // two new custom classes
            ObjectLayerFilter filter(ObjectLayerTable{true,true,false,false}); //that are annoying as fuck to use because you can't copy them for some reason

            // what the hell is the point of JPH::RayCast if I can't use it to raycast??
            physics_system.GetNarrowPhaseQuery().CastRay((JPH::RRayCast)raycast,result,broadFilter,filter,JPH::BodyFilter());

            if(result.mFraction < 1.0f) {
                float distance = dist * result.mFraction;
                vec3 point = ray.origin + ray.direction * distance;
                auto shape = physics_system.GetBodyInterface().GetShape(result.mBodyID).GetPtr();

                vec3 normal = {};
                {
                    JPH::BodyLockRead lock(physics_system.GetBodyLockInterface(), result.mBodyID);
                    if (lock.Succeeded()) // body_id may no longer be valid
                    {
                        const JPH::Body &body = lock.GetBody();

                        normal = Physics::toGlmVec(body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2,Physics::toJoltVec(point)));
                        
                    }
                }

                auto userData = physics_system.GetBodyInterface().GetUserData(result.mBodyID);
                Actor* actor = nullptr;
                int component = 0;
                if(userData != 0) {
                    auto userDataStruct = ActorUserData::decode(physics_system.GetBodyInterface().GetUserData(result.mBodyID));
                    actor = userDataStruct->actor;
                    component = userDataStruct->component;
                }
                return WorldRaycastHit(actor,RaycastHit(point,normal,distance),component);
            }

            return std::nullopt;
        }

        data_World save() {
            data_World data;
            iteratingActors++;
            for (auto& actor : actors)
            {
                data_ActorType type = actor->getActorDataType();
                if(type == data_ActorType::DONT_SAVE) {
                    continue;
                }
                data_ActorEntry data_entry;

                data_entry.type = type;
                auto buf = actor->createSaveBuffer();
                data_entry.name = actor->name;
                data_entry.data.reserve(buf.size());
                for (size_t i = 0; i < buf.size(); i++)
                {
                    data_entry.data.push_back(buf[i]);
                }
                
                data.actors.push_back(data_entry);
            }
            iteratingActors--;
            return data;
        }

        void clear() {

            iteratingActors++;
            for (auto& actor : actors)
            {
                if(!actor->destroyed) {
                    actor->destroy(this);

                }
            }
            iteratingActors--;

            actors.clear();
            spawnedActors.clear();
            contactListener.collisions.clear();
            camera = Camera();
            sinceLastStep = 0;

            
        }

        // this should be combined :shrug: to have like 1 param
        void load(data_World data,DataLoader& dataLoader) {

            clear();

            for (auto& data_actor : data.actors)
            {
                auto newActor = dataLoader.loadActor(data_actor);
                if(newActor == nullptr) {
                    Debug::warn("actor to load is null");
                    continue;
                }
                auto spawned_shared = spawn(std::move(newActor));
                // if(data_actor.type == data_ActorType::PLAYER) {
                //     auto possible_player = dynamic_cast<std::shared_ptr<Character>>()>(spawned_shared);
                // }
            }
        }


        Camera& getCamera() {
            return camera;
        }

};