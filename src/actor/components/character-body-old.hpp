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
#include <Jolt/Physics/Character/Character.h>

#include "physics/jolt-conversions.hpp"


#include "persistance/data-generic.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"

using glm::vec3,glm::mat3,glm::quat;

class Actor;


class CharacterBody {
    
    
    JPH::Character *character;
    vec3 velocity = {};
    ActorUserData userData;
    
    public:
        bool generateCollisionEvents = false;

        JPH::Character* getCharacter() {
            return character;
        }

        JPH::BodyID getBodyID() {
            if(character == nullptr) return JPH::BodyID(); //return invalid body ID
            return character->GetBodyID();
        }

        void setVelocity(vec3 v) {
            velocity = v;
        }

        vec3 getVelocity() {
            return velocity;
        }
        

        void spawn(World* world,Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {

            
            JPH::CharacterSettings settings = getDefaultCharacterSettings(actor,shape,position,rotation);

            spawn(world,actor,settings);
        }

        JPH::CharacterSettings getDefaultCharacterSettings(Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {
            JPH::CharacterSettings settings;

            settings.mGravityFactor = 1.0f;
            settings.mLayer = Layers::MOVING;
            settings.mShape = shape;
            settings.mFriction = 2.0f;
            settings.mEnhancedInternalEdgeRemoval = true;
            userData.actor = actor;
            userData.component = 0;
            userData.generateCollisionEvents = generateCollisionEvents;
            return settings;
        }
        
        void spawn(World* world,Actor* actor,JPH::CharacterSettings settings) {
            
            
            character = new JPH::Character(&settings,Physics::toJoltVec(actor->getPosition()),Physics::toJoltQuat(actor->getRotation()),ActorUserData::encode(&userData),&world->physics_system);

            character->AddToPhysicsSystem(JPH::EActivation::Activate);

            character->SetLinearVelocity(Physics::toJoltVec(velocity));
            
        }

        JPH::CharacterBase::EGroundState getGroundState() {
            return character->GetGroundState();
        }

        void prePhysics(World* world,vec3 position,quat rotation) {
            userData.generateCollisionEvents = generateCollisionEvents;
            
            character->SetPosition(Physics::toJoltVec(position),JPH::EActivation::DontActivate);
            character->SetRotation(Physics::toJoltQuat(rotation).Normalized(),JPH::EActivation::DontActivate);
            character->SetLinearVelocity(Physics::toJoltVec(velocity));
        }

        void postPhysics(World* world,vec3& position,quat& rotation) {
            position = Physics::toGlmVec(character->GetPosition());
            rotation = Physics::toGlmQuat(character->GetRotation().Normalized());
            velocity = Physics::toGlmVec(character->GetLinearVelocity());
            character->PostSimulation(0.005);
        }

        void destroy(World* world) {

            world->physics_system.GetBodyInterface().RemoveBody(character->GetBodyID());
            world->physics_system.GetBodyInterface().DestroyBody(character->GetBodyID());
        }

        data_Rigidbody save() {
            data_Rigidbody data{};
            data.velocity.set(velocity);
            data.angularVelocity.set(vec3(0.0f));
            return data;
        }

        void load(data_Rigidbody data) {
            velocity = data.velocity.toVec3();
        }

       

};