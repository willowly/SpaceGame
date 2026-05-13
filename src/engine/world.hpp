#pragma once

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
#include "physics/jolt-setup.hpp"

#include "persistance/data-world.hpp"
#include "persistance/data-loader.hpp"

#include "actor/components/gravity-well.hpp"

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

    vector<unique_ptr<Actor>> actors;
    vector<unique_ptr<Actor>> spawnedActors; //for when we spawn in the step
    map<ActorID,Actor*> actorIDMap;
    vec3 constantGravity = vec3(0,-5,0);

    vector<GravityWell*> gravityWells;

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

            Physics::initalizePhysicsGlobal();

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

        int nextID = 1;

        World() {
            setupPhysics();
        }

        template<typename T>
        T* spawn(unique_ptr<T> spawned) {

            
            auto* rawSpawned = spawned.get();
            spawnedActors.push_back(std::move(spawned));
            // raw spawned?

            rawSpawned->spawn(this); // do actor specific spawning code
            if(rawSpawned->id == Invalid_ActorID) { //maybe a bit jank but only add a custom id if there isn't already one
                rawSpawned->id = nextID; // possible this should be in the actual actor creation
                nextID++;
            } else {
                nextID = rawSpawned->id; //to make sure ids of other entities are unique
            }

            actorIDMap[rawSpawned->id] = rawSpawned;

            return rawSpawned;
        }

        template <typename T>
        T* getActor(ActorID id) {
            if(actorIDMap.contains(id)) {
                return dynamic_cast<T*>(actorIDMap.at(id));
            } else {
                return nullptr;
            }
        }

        float getInterpolationTime() {
            return sinceLastStep / stepDt;
        }

        vec3 getGravityVector(vec3 position) {
            vec3 gravity = {};
            for(auto well : gravityWells) {
                gravity += well->getGravityVector(position);
            }
            return gravity;
        }

        void addGravityWell(GravityWell* well) {
            if(well == nullptr) throw std::runtime_error("gravity well is null");
            if(std::find(gravityWells.begin(),gravityWells.end(),well) != gravityWells.end()) throw std::runtime_error("gravity well already exists");
            gravityWells.push_back(well);
        }

        void removeGravityWell(GravityWell* well) {
            if(well == nullptr) throw std::runtime_error("gravity well is null");
            auto iter = std::find(gravityWells.begin(),gravityWells.end(),well);
            if(iter == gravityWells.end())  
                throw std::runtime_error("gravity well does not exist");
            gravityWells.erase(iter);
        }

        JPH::TempAllocatorImpl& getAllocator() {
            if(temp_allocator == nullptr) {
                throw std::runtime_error("cannot get temp allocator");
            }
            return *temp_allocator;
        }

        std::vector<Actor*> getActors() {

            std::vector<Actor*> list;
            
            
            getActors(list);
            

            return list;

        }

        void getActors(std::vector<Actor*>& list) {

            list.clear();
            
            
            for (auto& actor : actors)
            {
                if(actor->destroyed) continue;
                list.push_back(actor.get());
                
            }

        }

        //probably should remove this, but its for getting the player. Ill think of a better way to handle this I guess
        template <typename T>
        ActorID getActorOfType() {
            
            for (auto& actor : actors)
            {
                std::shared_ptr<T> typed_actor = std::dynamic_pointer_cast<T>(actor);
                if(typed_actor != nullptr) {
                    
                    return typed_actor->id;
                }
                
            }

            return nullptr;

        }

        // this is for testing mainly
        template <typename T>
        std::vector<ActorID> getActorsOfType() {

            std::vector<ActorID> list;
            
            
            for (auto& actor : actors)
            {
                std::shared_ptr<T> typed_actor = std::dynamic_pointer_cast<T>(actor);
                if(typed_actor != nullptr) {
                    list.push_back(typed_actor->id);
                }
                
            }
            

            return list;

        }
        
        // do rendering, step and everything else
        void frame(Vulkan* vulkan,float dt) {
            
            ZoneScoped;
            
            float clock = glfwGetTime();
            addRenderables(vulkan,dt,sinceLastStep/stepDt);
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

        // do rendering, step and everything else
        void frameClient(Vulkan* vulkan,float dt) {
            
            ZoneScoped;
            
            float clock = glfwGetTime();
            addRenderables(vulkan,dt,1);
            eraseDestroyedActors();
            addSpawnedActors();
        }


        void addRenderables(Vulkan* vulkan,float dt,float interpolation) {
            ZoneScoped;
            for (auto& actor : actors)
            {
                actor->addRenderables(vulkan,dt,interpolation);
            }
            
        }

        void physicsStep(float dt) {
            
            ZoneScoped;

            {
                //ZoneScopedN("prePhysics")
                
                for (auto& actor : actors)
                {
                    actor->prePhysics(this);
                }
                
            }

            physics_system.SetGravity(JPH::Vec3(0.0,-10.0,0.0));

            physics_system.Update(dt,1,temp_allocator,jobSystem);

            {
                //ZoneScopedN("postPhysics")
                
                for (auto& actor : actors)
                {
                    actor->postPhysics(this);
                }
                
            }

            for (auto& collision : contactListener.collisions)
            {
                collision.actor->collisionStart(this,collision);
            }
            contactListener.collisions.clear();
        }

        void step(float dt) {
            ZoneScoped;
            
            
            for (auto& actor : actors)
            {
                if(actor->networkLocal) {
                    actor->step(this,dt);
                }
            }

            eraseDestroyedActors(); 
            
            addSpawnedActors();
            

            physicsStep(dt);

            
        }

        void eraseDestroyedActors() {
            // remove the ones marked as destroyed
            for (int i = actors.size() - 1; i >= 0; i--)
            {
                auto& actor = actors[i];
                if(actor->destroyed) {
                    actorIDMap.erase(actor->id);
                    actors.erase(actors.begin()+i);
                }
            }  
        }

        void addSpawnedActors() {
            // this needs to happen first, because newly spawned actors need to have prePhysics() and postPhysics() called on them as well, since they are already in the physic system
            for(auto& actor : spawnedActors) {
                actors.push_back(std::move(actor));
            }
            spawnedActors.clear();
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            //if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }

        std::vector<Actor*> overlapSphere(vec3 pos,float radius) {
            // JPH::ShapeCast shapecast();
            // JPH::SphereShape shape(radius);
            // physics_system.GetNarrowPhaseQuery().CollideShape(&shape,JPH::Vec3(1.0f,1.0f,1.0f),)

            std::vector<Actor*> results;

            
            for (auto& actor : actors)
            {
                if(glm::length2(actor->getPosition() - pos) < radius*radius) {
                    results.push_back(actor.get());
                }
            }
            

            return results;
            
        }

        std::optional<WorldRaycastHit> raycast(Ray ray,float dist,LayerMask mask = LayerMask::all(),const RaycastSettings& settings = RaycastSettings()) {
            //ZoneScoped;
            //std::optional<WorldRaycastHit> result = std::nullopt; 
            ray.direction = glm::normalize(ray.direction);

            
            JPH::RayCast raycast(Physics::toJoltVec(ray.origin),Physics::toJoltVec(ray.direction * dist));
            JPH::RayCastResult result;

            JPH::BodyFilter bodyFilter;

            // what the hell is the point of JPH::RayCast if I can't use it to raycast??
            physics_system.GetNarrowPhaseQuery().CastRay((JPH::RRayCast)raycast,result,mask.getBroadPhaseLayerFilter(),mask.getObjectLayerFilter(),settings.getBodyFilter());

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

        std::optional<Actor*> overlapBox(vec3 position,vec3 size,quat rotation = glm::identity<quat>()) {
            BroadPhaseLayerFilter broadFilter(BroadPhaseLayerTable{true,true,false}); 
            ObjectLayerFilter filter(ObjectLayerTable{true,true,true,false});

            std::cout << "overlapping box" << std::endl;
            Debug::drawCube(position,size,rotation,Color::red,0.2f);

            

            class OverlapCollector : public JPH::CollideShapeCollector
			{
                void AddHit(const JPH::CollideShapeResult &inResult) override
				{
                    auto value = system->GetBodyInterface().GetUserData(inResult.mBodyID2);
                    if(value != 0) {
                        auto userDataStruct = ActorUserData::decode(value);
                        if(userDataStruct->actor != nullptr) {
                            actor = userDataStruct->actor;
                            ForceEarlyOut();
                        }
                    }
                }
                public:
                    JPH::PhysicsSystem* system;
                    Actor* actor = nullptr;
                    OverlapCollector(JPH::PhysicsSystem* system) : system(system) {}
                
            };

            JPH::Mat44 mat = JPH::Mat44::sIdentity();
            mat = mat.PostTranslated(Physics::toJoltVec(position));
            mat = mat * mat.sRotation(Physics::toJoltQuat(rotation));
            OverlapCollector collector(&physics_system);
            physics_system.GetNarrowPhaseQuery().CollideShape(new JPH::BoxShape(Physics::toJoltVec(size*0.5f)),JPH::Vec3::sOne(),mat,JPH::CollideShapeSettings(),JPH::Vec3(0,0,0),collector,broadFilter,filter,JPH::BodyFilter(),JPH::ShapeFilter());
            if(collector.actor != nullptr) {
                return collector.actor;
            }
            return std::nullopt;
        }

        data_World save() {
            data_World data;
            
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
            
            return data;
        }

        void clear() {

            
            for (auto& actor : actors)
            {
                if(!actor->destroyed) {
                    actor->destroy(this);

                }
            }
            

            actors.clear();
            spawnedActors.clear();
            actorIDMap.clear();
            contactListener.collisions.clear();
            camera = Camera();
            sinceLastStep = 0;

            
        }

        
        void load(data_World data,DataLoader& dataLoader) {

            clear();

            for (auto& data_actor : data.actors)
            {
                loadActor(data_actor,dataLoader);
                // if(data_actor.type == data_ActorType::PLAYER) {
                //     auto possible_player = dynamic_cast<std::shared_ptr<Character>>()>(spawned_shared);
                // }
            }
        }

        void destroyActor(ActorID id) {
            auto actor = getActor<Actor>(id);
            if(actor != nullptr) {
                actor->destroy(this);
            }
        }

        Actor* loadActor(data_ActorEntry data_entry,DataLoader& dataLoader) {
            auto newActor = dataLoader.loadActor(data_entry);
            if(newActor == nullptr) {
                Debug::warn("actor to load is null");
                return nullptr;
            }
            return spawn(std::move(newActor));
        }


        Camera& getCamera() {
            return camera;
        }

};

// template <>
// Actor* World::getActor<Actor>(ActorID id) {
//     if(actorIDMap.contains(id)) {
//         return actorIDMap.at(id);
//     } else {
//         return nullptr;
//     }
// }