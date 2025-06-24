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

#include <reactphysics3d/reactphysics3d.h>

#include "helper/collision-helper.hpp"

using CollisionHelper::RaycastHit,CollisionHelper::intersectRayBox,std::optional,glm::vec3,CollisionHelper::Ray,MathHelper::lerp;

class PhysicsActor : public Actor {

    private:
        glm::vec3 scale = vec3(1.0f); //this is temporary!!!

        rp3d::RigidBody* body;

    public:

        PhysicsActor(Model* model,Material* material) : Actor(model,material) {
            
        }
        PhysicsActor() : PhysicsActor(nullptr,nullptr){
            
        }

        void step(float dt) {
            body->setTransform(getPhysicsTransform());
        }

        virtual void render(Camera& camera) {
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

        void addToWorld(rp3d::PhysicsWorld* world,rp3d::PhysicsCommon* common) {
            body = world->createRigidBody(getPhysicsTransform());
            body->addCollider(common->createBoxShape(rp3d::Vector3(scale.x,scale.y,scale.z)),rp3d::Transform());
        }

        void updatePhysicsRepresentation() {
            body->setTransform(getPhysicsTransform());
        }

        void updateFromPhysicsRepresentation() {
            setPhysicsTransform(body->getTransform());
        }

        // these functions are not named properly
        rp3d::Transform getPhysicsTransform() {
            rp3d::Vector3 rp3dPosition(position.x,position.y,position.z);
            rp3d::Quaternion p3dRotation(rotation.x,rotation.y,rotation.z,rotation.w);
            return rp3d::Transform(rp3dPosition,p3dRotation);
        }

        void setPhysicsTransform(rp3d::Transform transform) {
            rp3d::Vector3 p = transform.getPosition();
            rp3d::Quaternion r = transform.getOrientation();
            position.x = p.x;
            position.y = p.y;
            position.z = p.z;
            rotation.x = r.x;
            rotation.y = r.y;
            rotation.z = r.z;
            rotation.w = r.w;
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





        float getMass() {
            return body->getMass();
        }
        void setMass(float mass) {
            body->setMass(mass);
        }

        optional<RaycastHit> raycast(Ray ray) {
            return intersectRayBox(position,scale,rotation,ray);
        }

        static void transformInertiaTensor(glm::mat3 &iitWorld, const glm::quat &q, const glm::mat3 &iitBody)
        {

            const float* rotMatData = glm::value_ptr(glm::toMat4(q));
            const float* ittBodyData = glm::value_ptr(iitBody);

            float t4 = rotMatData[0]*ittBodyData[0]+
            rotMatData[1]*ittBodyData[3]+
            rotMatData[2]*ittBodyData[6];
            float t9 = rotMatData[0]*ittBodyData[1]+
            rotMatData[1]*ittBodyData[4]+
            rotMatData[2]*ittBodyData[7];
            float t14 = rotMatData[0]*ittBodyData[2]+
            rotMatData[1]*ittBodyData[5]+
            rotMatData[2]*ittBodyData[8];
            float t28 = rotMatData[4]*ittBodyData[0]+
            rotMatData[5]*ittBodyData[3]+
            rotMatData[6]*ittBodyData[6];
            float t33 = rotMatData[4]*ittBodyData[1]+
            rotMatData[5]*ittBodyData[4]+
            rotMatData[6]*ittBodyData[7];
            float t38 = rotMatData[4]*ittBodyData[2]+
            rotMatData[5]*ittBodyData[5]+
            rotMatData[6]*ittBodyData[8];
            float t52 = rotMatData[8]*ittBodyData[0]+
            rotMatData[9]*ittBodyData[3]+
            rotMatData[10]*ittBodyData[6];
            float t57 = rotMatData[8]*ittBodyData[1]+
            rotMatData[9]*ittBodyData[4]+
            rotMatData[10]*ittBodyData[7];
            float t62 = rotMatData[8]*ittBodyData[2]+
            rotMatData[9]*ittBodyData[5]+
            rotMatData[10]*ittBodyData[8];

            float* ittWorldData = glm::value_ptr(iitWorld);

            ittWorldData[0] = t4*rotMatData[0]+
            t9*rotMatData[1]+
            t14*rotMatData[2];
            ittWorldData[1] = t4*rotMatData[4]+
            t9*rotMatData[5]+
            t14*rotMatData[6];
            ittWorldData[2] = t4*rotMatData[8]+
            t9*rotMatData[9]+
            t14*rotMatData[10];
            ittWorldData[3] = t28*rotMatData[0]+
            t33*rotMatData[1]+
            t38*rotMatData[2];
            ittWorldData[4] = t28*rotMatData[4]+
            t33*rotMatData[5]+
            t38*rotMatData[6];
            ittWorldData[5] = t28*rotMatData[8]+
            t33*rotMatData[9]+
            t38*rotMatData[10];
            ittWorldData[6] = t52*rotMatData[0]+
            t57*rotMatData[1]+
            t62*rotMatData[2];
            ittWorldData[7] = t52*rotMatData[4]+
            t57*rotMatData[5]+
            t62*rotMatData[6];
            ittWorldData[8] = t52*rotMatData[8]+
            t57*rotMatData[9]+
            t62*rotMatData[10];
        }
        

};