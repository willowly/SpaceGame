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
#include "helper/physics-helper.hpp"

#include "helper/collision-helper.hpp"

using CollisionHelper::RaycastHit,CollisionHelper::intersectRayBox,std::optional,glm::vec3,CollisionHelper::Ray,MathHelper::lerp;

class RigidbodyActor : public Actor {

    protected:
        glm::vec3 scale = vec3(1.0f); //this is temporary!!!

        rp3d::PhysicsCommon* common;
        
    public:
        vec3 velocity = vec3(0);
        rp3d::RigidBody* body;
        ActorUserData userData;

        RigidbodyActor(Model* model,Material* material) : Actor(model,material) {
            
        }
        RigidbodyActor() : RigidbodyActor(nullptr,nullptr){
            
        }

        virtual void step(float dt,World* world) {
            
        }

        virtual void render(Camera& camera,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            if(material == nullptr) {
                std::cout << "null material" << std::endl;
            }
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix,position);
            matrix = matrix * glm::toMat4(rotation);
            matrix = glm::scale(matrix,vec3(modelScale) * scale);
            model->render(matrix,camera,*material);
        }

        // these functions are not named properly
        rp3d::Transform getPhysicsTransform() {
            rp3d::Vector3 rp3dPosition(position.x,position.y,position.z);
            rp3d::Quaternion p3dRotation(rotation.x,rotation.y,rotation.z,rotation.w);
            return rp3d::Transform(rp3dPosition,p3dRotation);
        }

        void setScale(vec3 scale) {
            this->scale = scale;
        }

        vec3 getScale() {
            return scale;
        }

        void applyForce(vec3 force,vec3 point) {
            body->applyWorldForceAtWorldPosition(
                PhysicsHelper::toRp3dVector(force),
                PhysicsHelper::toRp3dVector(point)
            );
        }

        void addToPhysicsWorld(rp3d::PhysicsWorld* world,rp3d::PhysicsCommon* common) {
            userData.actor = this;
            body = world->createRigidBody(getPhysicsTransform());
            body->enableGravity(false);
            body->setUserData(&userData);
            addCollisionShapes(common);
            this->common = common;
        }

        void updatePhysicsRepresentation() {
            body->setTransform(getPhysicsTransform());
            body->setLinearVelocity(PhysicsHelper::toRp3dVector(velocity));
        }

        void updateFromPhysicsRepresentation() {
            auto transform = (body->getTransform());
            position = PhysicsHelper::toGlmVector(transform.getPosition());
            rotation = PhysicsHelper::toGlmQuaternion(transform.getOrientation());
            velocity = PhysicsHelper::toGlmVector(body->getLinearVelocity());
        }

        virtual void addCollisionShapes(rp3d::PhysicsCommon* common) {
            auto collider = body->addCollider(common->createBoxShape(rp3d::Vector3(scale.x,scale.y,scale.z)),rp3d::Transform());
            collider->getMaterial().setBounciness(0.0);
        }


        float getMass() {
            return body->getMass();
        }
        void setMass(float mass) {
            body->setMass(mass);
        }

        

};