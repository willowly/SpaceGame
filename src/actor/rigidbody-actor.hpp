#pragma once
#include "actor.hpp"
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

using std::optional,glm::vec3,glm::mat3,MathHelper::lerp;

struct data_RigidbodyActor {
    data_Actor actor;
    data_vec3 velocity;
    data_vec3 angularVelocity;
};

class RigidbodyActor : public Actor {

    protected:
        


        RigidbodyActor(Mesh<Vertex>* mesh,Material material) : Actor() {
            model = mesh;
            this->material = material;
            
        }
        
    public:

        //float mass;

        vec3 velocity = {};
        vec3 angularVelocity = {};

        JPH::Body *body;

        static std::unique_ptr<RigidbodyActor> makeDefaultPrototype() {
            auto ptr = new RigidbodyActor(nullptr,Material::none);
            return std::unique_ptr<RigidbodyActor>(ptr);
        }

        static std::unique_ptr<RigidbodyActor> makeInstance(RigidbodyActor* prototype,vec3 position,quat rotation,vec3 velocity,vec3 angularVelocity){
            auto instance = makeInstanceFromPrototype<RigidbodyActor>(prototype);
            instance->position = position;
            instance->rotation = rotation;
            instance->velocity = velocity;
            instance->angularVelocity = angularVelocity;

            // update body
            return instance;
        }

        virtual void spawn(World* world) {

            Physics::initalizePhysicsGlobal();

            JPH::BodyCreationSettings bodySettings(new JPH::BoxShape(JPH::Vec3(1.0f, 1.0f, 1.0f)), Physics::toJoltVec(position), Physics::toJoltQuat(position), JPH::EMotionType::Dynamic, Layers::MOVING);

            bodySettings.mGravityFactor = 0.0f;
            body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
            world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);

            body->SetLinearVelocity(Physics::toJoltVec(velocity));
            body->SetAngularVelocity(Physics::toJoltVec(angularVelocity));
            body->SetRestitution(0.1f);
            body->SetFriction(2.0);


        }

        void step(World* world,float dt) {

            
            
        }

        virtual void prePhysics(World* world) {
            world->physics_system.GetBodyInterface().SetPosition(body->GetID(),Physics::toJoltVec(position),JPH::EActivation::DontActivate);
            world->physics_system.GetBodyInterface().SetRotation(body->GetID(),Physics::toJoltQuat(rotation),JPH::EActivation::DontActivate);
            world->physics_system.GetBodyInterface().SetLinearVelocity(body->GetID(),Physics::toJoltVec(velocity));
            world->physics_system.GetBodyInterface().SetAngularVelocity(body->GetID(),Physics::toJoltVec(angularVelocity));
        }

        virtual void postPhysics(World* world) {
            position = Physics::toGlmVec(body->GetPosition());
            rotation = Physics::toGlmQuat(body->GetRotation().Normalized());
            velocity = Physics::toGlmVec(body->GetLinearVelocity());
            angularVelocity = Physics::toGlmVec(body->GetAngularVelocity());
        }

        
        

        void destroy(World* world) {
            world->physics_system.GetBodyInterface().RemoveBody(body->GetID());
            world->physics_system.GetBodyInterface().DestroyBody(body->GetID());
            Actor::destroy(world);
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix,position);
            matrix = matrix * glm::toMat4(rotation);
            matrix = glm::scale(matrix,vec3(modelScale));
            model->addToRender(vulkan,material,matrix);
        }

        data_ActorType getActorDataType() {
            return data_ActorType::PHYSICS;
        }

        virtual std::vector<std::uint8_t> createSaveBuffer() {
            auto data = save();
            auto buf = cista::serialize(data);
            return buf;
        }

        data_RigidbodyActor save() {
            data_RigidbodyActor data;
            data.actor = Actor::save();
            data.velocity.set(velocity);
            data.angularVelocity.set(angularVelocity);
            return data;
        }

        void load(const data_RigidbodyActor& data) {
           Actor::load(data.actor);
           velocity = data.velocity.toVec3();
           angularVelocity = data.angularVelocity.toVec3();
        }

        static std::unique_ptr<Actor> makeInstanceFromSave(data_RigidbodyActor& data,RigidbodyActor* prototype) {

            if(prototype == nullptr) throw std::runtime_error("prototype is null");
            auto actor = makeInstanceFromPrototype(prototype);
            actor->load(data);

            std::cout << "LOADING PHYSICS ACTOR" << std::endl;

            return actor;
        }

        // virtual optional<RaycastHit> raycast(Ray ray,float distance) {
        //     return Physics::intersectRayBox(position,vec3(1.0f),rotation,ray);
        // }

        

};