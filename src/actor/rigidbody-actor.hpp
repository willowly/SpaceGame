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

#include "helper/jolt-helper.hpp"

using std::optional,glm::vec3,glm::mat3,MathHelper::lerp;

class RigidbodyActor : public Actor {

    protected:
        


        RigidbodyActor(Mesh<Vertex>* mesh,Material material) : Actor() {
            model = mesh;
            this->material = material;
            
        }
        
    public:

        //float mass;

        vec3 velocity;
        vec3 angularVelocity;

        JPH::Body *body;
        
        virtual void setPosition(vec3 position) {
            this->position = position;
            //body->move (JHLP::toJoltVec(position));
        }
        virtual void setRotation(quat rotation) {
            this->rotation = rotation;
            //body->SetRotation(JHLP::toJoltQuat(rotation));
        }

        static std::unique_ptr<RigidbodyActor> makeDefaultPrototype() {
            auto ptr = new RigidbodyActor(nullptr,Material::none);
            return std::unique_ptr<RigidbodyActor>(ptr);
        }

        static std::unique_ptr<RigidbodyActor> makeInstance(RigidbodyActor* prototype,vec3 position,quat rotation,vec3 velocity,vec3 angularVelocity){
            auto instance = makeInstanceFromPrototype<RigidbodyActor>(prototype);
            instance->position = position;
            instance->rotation = rotation;

            // update body
            return instance;
        }

        virtual void spawn(World* world) {

            JPH::BodyCreationSettings bodySettings(new JPH::BoxShape(JPH::Vec3(1.0f, 1.0f, 1.0f)), JHLP::toJoltVec(position), JHLP::toJoltQuat(position), JPH::EMotionType::Dynamic, Layers::MOVING);

            body = world->physics_system.GetBodyInterfaceNoLock().CreateBody(bodySettings);
            world->physics_system.GetBodyInterfaceNoLock().AddBody(body->GetID(),JPH::EActivation::Activate);

            body->SetLinearVelocity(JHLP::toJoltVec(velocity));
            body->SetAngularVelocity(JHLP::toJoltVec(angularVelocity));


        }

        virtual void step(World* world,float dt) {

            position = JHLP::toGlmVec(body->GetPosition());
            rotation = JHLP::toGlmQuat(body->GetRotation());

            std::cout << StringHelper::toString(position) << std::endl;
            std::cout << StringHelper::toString(JHLP::toGlmVec(body->GetLinearVelocity())) << std::endl;
            
            // we aren't doing any modifications just yet

            // body.setPosition(position);
            // body.setRotation(rotation);
            
        }
        

        virtual void destroy(World* world) {
            //world->removePhysicsBody(&body);
            Actor::destroy(world);
        }

        virtual void addRenderables(Vulkan* vulkan,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix,position);
            matrix = matrix * glm::toMat4(rotation);
            matrix = glm::scale(matrix,vec3(modelScale));
            model->addToRender(vulkan,material,matrix);
        }

        // virtual optional<RaycastHit> raycast(Ray ray,float distance) {
        //     return Physics::intersectRayBox(position,vec3(1.0f),rotation,ray);
        // }

        

};