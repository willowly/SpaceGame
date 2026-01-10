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

using std::optional,glm::vec3,glm::mat3,MathHelper::lerp;

class RigidbodyActor : public Actor {

    protected:
        


        RigidbodyActor(Mesh<Vertex>* mesh,Material material,float mass) : Actor(), body(mass) {
            model = mesh;
            this->material = material;
        }
        
    public:

        PhysicsBody body;
        
        virtual void setPosition(vec3 position) {
            this->position = position;
            body.setPosition(position);
        }
        virtual void setRotation(quat rotation) {
            this->rotation = rotation;
            body.setRotation(rotation);
        }

        static std::unique_ptr<RigidbodyActor> makeDefaultPrototype() {
            auto ptr = new RigidbodyActor(nullptr,Material::none,1);
            return std::unique_ptr<RigidbodyActor>(ptr);
        }

        static std::unique_ptr<RigidbodyActor> makeInstance(RigidbodyActor* prototype,vec3 position,quat rotation,vec3 velocity,vec3 angularVelocity){
            auto instance = makeInstanceFromPrototype<RigidbodyActor>(prototype);
            instance->position = position;
            instance->rotation = rotation;

            // update body
            instance->body.setPosition(position);
            instance->body.setRotation(rotation);
            instance->body.setVelocity(velocity);
            instance->body.setAngularVelocity(angularVelocity);
            return instance;
        }

        virtual void spawn(World* world) {
            world->addPhysicsBody(&body);
        }

        virtual void step(World* world,float dt) {

            position = body.getPosition();
            rotation = body.getRotation();
            
            // we aren't doing any modifications just yet

            body.setPosition(position);
            body.setRotation(rotation);
            
        }
        

        virtual void destroy(World* world) {
            world->removePhysicsBody(&body);
            Actor::destroy(world);
        }

        virtual void addRenderables(Vulkan* vulkan,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix,position);
            matrix = matrix * glm::toMat4(rotation);
            matrix = glm::scale(matrix,vec3(modelScale) * body.getScale());
            model->addToRender(vulkan,material,matrix);
        }

        // virtual optional<RaycastHit> raycast(Ray ray,float distance) {
        //     return Physics::intersectRayBox(position,vec3(1.0f),rotation,ray);
        // }

        

};