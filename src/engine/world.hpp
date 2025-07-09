#pragma once

#include "GLFW/glfw3.h"

#include "actor/actor.hpp"
#include <memory>
#include "actor/rigidbody-actor.hpp"
#include "helper/collision-helper.hpp"

#include <reactphysics3d/reactphysics3d.h>

using glm::vec3, glm::quat,std::unique_ptr;

#define WORLD
class World {

    vector<unique_ptr<Actor>> actors;
    vec3 constantGravity = vec3(0,-15,0);

    rp3d::PhysicsWorld* physicsWorld;
    rp3d::PhysicsCommon physicsCommon;

    Camera camera;

    float sinceLastStep;
   
    class DebugCallback : public rp3d::RaycastCallback {
    
        public:
    
        virtual rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) {
    
            // Display the world hit point coordinates
            Debug::drawPoint(ph::toGlmVector(info.worldPoint));

            std::cout << "Hit point : " << StringHelper::toString(ph::toGlmVector(info.worldPoint)) << std::endl;
    
            return info.hitFraction;
        }
    };
        

    public:

        float stepDt = 0.02;

        float stepProcessMs;
        float renderProcessMs;

        rp3d::RigidBody* floor;

        World() {
            physicsWorld = physicsCommon.createPhysicsWorld();
            // floor = physicsWorld->createRigidBody(rp3d::Transform(rp3d::Vector3(0,0,0),rp3d::Quaternion::identity()));
            // floor->addCollider(physicsCommon.createBoxShape(rp3d::Vector3(10,1,10)),rp3d::Transform(rp3d::Vector3(0,-0.5,0),rp3d::Quaternion::identity()));
            // floor->setType(rp3d::BodyType::STATIC);
        }

        template <typename T>
        T* spawn(T* prototype,vec3 position,quat rotation) {
            Actor* actorPointer = new T(*prototype);
            actors.push_back(std::unique_ptr<Actor>(actorPointer));
            actors[actors.size()-1]->position = position;
            actors[actors.size()-1]->rotation = rotation;
            actors[actors.size()-1]->addToPhysicsWorld(physicsWorld,&physicsCommon);
            return dynamic_cast<T*>(actorPointer);
        }

        void destroy(Actor* actor) {

        }


        vec3 getGravityVector(vec3 position) {
            return constantGravity;
        }
        
        // do rendering, step and everything else
        void frame(float dt) {
            
            float clock = glfwGetTime();
            render(camera,dt);
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

        void render(Camera& camera,float dt) {
            for (auto& actor : actors)
            {
                actor->render(camera,dt);
            }
            
        }

        void step(float dt) {
            for (auto& actor : actors)
            {
                actor->step(dt,this);
                actor->updatePhysicsRepresentation();
            }
            physicsStep(dt);
            for(auto& actor : actors) {
                actor->updateFromPhysicsRepresentation();
            }
        }

        void physicsStep(float dt) {
            physicsWorld->update(dt);
        }

        void applyGravityIfEnabled(Actor* actor,float dt) {
            //if(actor->useGravity) actor->velocity += getGravityVector(actor->position) * dt;
        }

        void raycast(vec3 position,vec3 direction,float distance,rp3d::RaycastCallback* callback) {
            physicsWorld->raycast(rp3d::Ray(ph::toRp3dVector(position),ph::toRp3dVector(position + glm::normalize(direction) * distance),1.0),callback);
        }

        Camera& getCamera() {
            return camera;
        }



        rp3d::PhysicsWorld* getPhysicsWorld() {
            return physicsWorld;
        }

        rp3d::PhysicsCommon* getPhysicsCommon() {
            return &physicsCommon;
        }

};