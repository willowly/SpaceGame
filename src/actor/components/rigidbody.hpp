#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>
#include <string>
#include <iostream>
#include "helper/math-helper.hpp"
#include "helper/string-helper.hpp"
#include "engine/world.hpp"
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

#include "physics/jolt-conversions.hpp"


#include "persistance/data-generic.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"

using glm::vec3,glm::mat3,glm::quat;

class Actor;


class Rigidbody {
    
    
    JPH::Body *body = nullptr;
    vec3 velocity = {};
    vec3 angularVelocityRads = {};
    ActorUserData userData;
    
    public:
        bool generateCollisionEvents = false;
        bool useAngularVelocity = true; //rn just for overriding for the player

        JPH::Body* getBody() {
            return body;
        }

        JPH::BodyID getBodyID() {
            if(body == nullptr) return JPH::BodyID(); //return invalid body ID
            return body->GetID();
        }

        void setVelocity(vec3 v) {
            velocity = v;
        }

        vec3 getVelocity() {
            return velocity;
        }

        void setAngularVelocity(vec3 v) {
            angularVelocityRads = glm::radians(v);
        }

        vec3 getAngularVelocity() {
            return glm::degrees(angularVelocityRads);
        }

        vec3 getAngularVelocityRadians() {
            return angularVelocityRads;
        }
        

        void spawn(World* world,Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {
            JPH::BodyCreationSettings bodySettings = getDefaultBodySettings(actor,shape,position,rotation);

            spawn(world,actor,bodySettings);
        }

        JPH::BodyCreationSettings getDefaultBodySettings(Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {
            JPH::BodyCreationSettings bodySettings(shape, Physics::toJoltVec(position), Physics::toJoltQuat(position), JPH::EMotionType::Dynamic, Layers::MOVING);
            bodySettings.mGravityFactor = 0.0f;
            userData.actor = actor;
            userData.component = 0;
            userData.generateCollisionEvents = generateCollisionEvents;
            bodySettings.mUserData = ActorUserData::encode(&userData);
            return bodySettings;
        }
        
        void spawn(World* world,Actor* actor,JPH::BodyCreationSettings bodySettings) {
            
            body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
            
            world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);

            if(!body->IsStatic()) { 
                body->SetLinearVelocity(Physics::toJoltVec(velocity));
                body->SetAngularVelocity(Physics::toJoltVec(angularVelocityRads));
            }
            body->SetRestitution(0.1f);
            body->SetFriction(2.0f);
            
        }

        void prePhysics(World* world,vec3 position,quat rotation) {
            userData.generateCollisionEvents = generateCollisionEvents;
            world->physics_system.GetBodyInterface().SetPosition(body->GetID(),Physics::toJoltVec(position),JPH::EActivation::DontActivate);
            world->physics_system.GetBodyInterface().SetRotation(body->GetID(),Physics::toJoltQuat(rotation).Normalized(),JPH::EActivation::DontActivate);
            if(!body->IsStatic()) { 
                world->physics_system.GetBodyInterface().SetLinearVelocity(body->GetID(),Physics::toJoltVec(velocity));
                world->physics_system.GetBodyInterface().SetAngularVelocity(body->GetID(),Physics::toJoltVec(angularVelocityRads));
            }
        }

        void postPhysics(World* world,vec3& position,quat& rotation) {
            position = Physics::toGlmVec(body->GetPosition());
            rotation = Physics::toGlmQuat(body->GetRotation().Normalized());
            if(!body->IsStatic()) { 
                velocity = Physics::toGlmVec(body->GetLinearVelocity());
                angularVelocityRads = Physics::toGlmVec(body->GetAngularVelocity());
            }
        }

        void applyGravity(World* world,vec3 position,float dt) {
            velocity += world->getGravityVector(position) * dt;
        }

        void destroy(World* world) {

            world->physics_system.GetBodyInterface().RemoveBody(body->GetID());
            world->physics_system.GetBodyInterface().DestroyBody(body->GetID());
        }

        data_Rigidbody save() {
            data_Rigidbody data{};
            data.velocity.set(velocity);
            data.angularVelocity.set(angularVelocityRads);
            return data;
        }

        void load(data_Rigidbody data) {
            velocity = data.velocity.toVec3();
            angularVelocityRads = data.angularVelocity.toVec3();
        }

       

};