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
#include <Jolt/Physics/Character/CharacterVirtual.h>

#include "physics/jolt-conversions.hpp"


#include "persistance/data-generic.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"

using glm::vec3,glm::mat3,glm::quat;

class Actor;


class CharacterBody {
    
    
    JPH::CharacterVirtual *character = nullptr;
    vec3 velocity = {};
    ActorUserData userData;
    
    public:
        bool generateCollisionEvents = false;

        JPH::CharacterVirtual* getCharacter() {
            return character;
        }

        // JPH::BodyID getBodyID() {
        //     if(character == nullptr) return JPH::BodyID(); //return invalid body ID
        //     return character->GetBodyID();
        // }

        void setVelocity(vec3 v) {
            velocity = v;
        }

        vec3 getVelocity() {
            return velocity;
        }
        

        void spawn(World* world,Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {

            
            JPH::CharacterVirtualSettings settings = getDefaultCharacterSettings(actor,shape,position,rotation);

            spawn(world,actor,settings);
        }

        JPH::CharacterVirtualSettings getDefaultCharacterSettings(Actor* actor,JPH::Shape* shape,vec3 position,quat rotation) {
            JPH::CharacterVirtualSettings settings;

            settings.mShape = shape;
            settings.mInnerBodyShape = shape;
            settings.mInnerBodyLayer = Layers::PLAYER;
            settings.mEnhancedInternalEdgeRemoval = true;
            userData.actor = actor;
            userData.component = 0;
            userData.generateCollisionEvents = generateCollisionEvents;
            return settings;
        }
        
        void spawn(World* world,Actor* actor,JPH::CharacterVirtualSettings settings) {
            
            
            character = new JPH::CharacterVirtual(&settings,Physics::toJoltVec(actor->getPosition()),Physics::toJoltQuat(actor->getRotation()),ActorUserData::encode(&userData),&world->physics_system);

            world->physics_system.GetBodyInterface().SetUserData(character->GetInnerBodyID(),ActorUserData::encode(&userData));

            character->SetLinearVelocity(Physics::toJoltVec(velocity));

            
        }

        JPH::CharacterBase::EGroundState getGroundState() {
            return character->GetGroundState();
        }

        void update(World* world,Actor* actor,vec3& position,quat& rotation,vec3 gravity,float dt) {
            userData.generateCollisionEvents = generateCollisionEvents;
            
            character->SetPosition(Physics::toJoltVec(position));
            character->SetRotation(Physics::toJoltQuat(rotation).Normalized());
            character->SetLinearVelocity(Physics::toJoltVec(velocity + (gravity * dt)));
            character->SetUp(Physics::toJoltVec(rotation * vec3(0,1,0)));

            
            BroadPhaseLayerFilter broadFilter(BroadPhaseLayerTable{true,true,false});
            ObjectLayerFilter objectFilter(ObjectLayerTable{true,true,true,false,false});

            bool wasSupported = character->IsSupported();

            JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
            
            character->ExtendedUpdate(dt,Physics::toJoltVec(gravity),updateSettings,broadFilter,objectFilter,JPH::BodyFilter(),JPH::ShapeFilter(),world->getAllocator());
            position = Physics::toGlmVec(character->GetPosition());
            rotation = Physics::toGlmQuat(character->GetRotation().Normalized());
            velocity = Physics::toGlmVec(character->GetLinearVelocity());

            // if(!character->IsSupported() && wasSupported) {
            //     float velocity = (mPosition - old_position).Dot(mUp) / dt;
            //     if (velocity <= 1.0e-6f)
            //         character->StickToFloor(stickToFloorStepDown, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inShapeFilter, inAllocator);
            // }
            
            for(auto& contact : character->GetActiveContacts()) {

                if(!contact.mHadCollision) {
                    continue;
                }
                // if(generateCollisionEvents) {
                //     ActorUserData* userData = ActorUserData::decode(contact.mUserData);
                //     if(userData != nullptr && userData->actor != nullptr) {
                //         if(userData->generateCollisionEvents) {
                //             Collision collision{
                //                 .actor = userData->actor,
                //                 .body = inBody1.GetID(),
                //                 .otherActor = actor2,
                //                 .otherBody = inBody2.GetID(),
                //                 .inManifold = inManifold,
                //             };
                //             userData->actor->collisionStart(world,contact);
                //         }
                        
                //     }
                // }
                JPH::Vec3 normal = contact.mSurfaceNormal;
                if(!character->IsSlopeTooSteep(normal)) {
                    normal = Physics::toJoltQuat(rotation) * JPH::Vec3(0,1,0);
                    // std::cout << "unsloped contact ";
                } else {
                    // std::cout << "sloped contact: ";
                }
                // std::cout << StringHelper::toString(Physics::toGlmVec(normal)) << ": ";
                auto delta = glm::dot(-velocity,Physics::toGlmVec(normal));
                if(delta > -0.016) { // only allow in the direction of the normal
                    velocity += delta * Physics::toGlmVec(normal);
                }
                // std::cout << StringHelper::toString(delta) << std::endl;
            }
            
        }


        void destroy(World* world) {
            assert(character != nullptr);
            world->physics_system.GetBodyInterface().RemoveBody(character->GetInnerBodyID());
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