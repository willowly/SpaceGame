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
#include "engine/world.hpp"

#include "helper/collision-helper.hpp"

using Physics::intersectRayBox,std::optional,glm::vec3,glm::mat3,MathHelper::lerp;

class RigidbodyActor : public Actor {

    protected:
        glm::vec3 scale = vec3(1.0f); //this is temporary!!!
        
    public:
        vec3 velocity = vec3(0);
        //vec3 accumVelocity = vec3(0);
        vec3 angularVelocity = vec3(0);
        //vec3 accumAngularVelocity = vec3(0);
        float inverseMass = 1;
        bool useGravity = true;
        float restitution = 0.0f;

        mat3 localInverseInertiaTensor;
        mat3 worldInverseInertiaTensor;

        RigidbodyActor(Model* model,Material* material) : Actor(model,material) {
            float mass = getMass();
            vec3 d = scale * 2.0f;
            vec3 d2 = vec3(d.x*d.x,d.y*d.y,d.z*d.z);
            localInverseInertiaTensor = glm::inverse(mat3(
                vec3((d2.y*d2.z)*mass/12,0,0),
                vec3(0,(d2.x*d2.z)*mass/12,0),
                vec3(0,0,(d2.x*d2.y)*mass/12)
            ));
            transformInertiaTensor(worldInverseInertiaTensor,rotation,localInverseInertiaTensor);
        }

        RigidbodyActor() : RigidbodyActor(nullptr,nullptr){
            
        }

        virtual void step(float dt,World* world) {
            // velocity += accumVelocity;
            // accumVelocity = vec3(0);
            // angularVelocity += accumAngularVelocity;
            // accumAngularVelocity = vec3(0);

            if(useGravity) velocity += world->getGravityVector(position) * dt;
            position += velocity * dt;
            //rotation = glm::quat(angularVelocity * dt) * rotation;
            rotation += dt * 0.5f * glm::quat(0,angularVelocity.x,angularVelocity.y,angularVelocity.z) * rotation;
            rotation = glm::normalize(rotation);
            transformInertiaTensor(worldInverseInertiaTensor,rotation,localInverseInertiaTensor);
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

        void setScale(vec3 scale) {
            this->scale = scale;
        }

        vec3 getScale() {
            return scale;
        }

        void applyForce(vec3 force,vec3 point) {
            vec3 delta = point - position;
            vec3 torque = glm::cross(delta,force);
            applyTorque(torque);
            applyForce(force);
        }

        void applyForce(vec3 force) {
            velocity += force * inverseMass;
        }

        void applyTorque(vec3 torque) {
            angularVelocity += worldInverseInertiaTensor * torque;
        }

        vec3 getVelocityAtPointRelative(vec3 point) {
            return glm::cross(angularVelocity,point) + velocity;
        }

        virtual void addCollisionShapes(rp3d::PhysicsCommon* common) {
            
        }

        optional<RaycastHit> raycast(Ray ray) {
            return intersectRayBox(position,vec3(1.0f),rotation,ray);
        }

        void collideWithPlane() {
            std::vector<Contact> contacts;
            generateContacts(contacts);
            if(contacts.size() == 0) return;
            
            for (int i = 0; i <= 10; i++)
            {
                float greatestContactForce = getContactForce(contacts[0]);
                Contact& worst = contacts[0];
                for (Contact& contact : contacts)
                {
                    float force = getContactForce(contact);
                    if(force > greatestContactForce) {
                        worst = contact;
                        greatestContactForce = force;
                    }
                }
                if(greatestContactForce > 0.0001) {
                    std::cout << greatestContactForce << std::endl;
                    Debug::drawRay(transformPoint(worst.point),greatestContactForce * vec3(0,1,0));
                    applyForce(greatestContactForce * vec3(0,1,0),transformPoint(worst.point));
                }
            }
 
            for (int i = 0; i <= 10; i++)
            {
                Contact& worst = contacts[0];
                for (Contact& contact : contacts)
                {
                    if(contact.point.y < worst.point.y) {
                        worst = contact;
                    }
                }
                resolveContactPosition(worst);
            }
            //position += deltaPosition;
            
        }

        float getContactForce(Contact& contact) {
            vec3 contactPointWorld = transformPoint(contact.point);
            vec3 contactPointDelta = contactPointWorld - position;
            vec3 torquePerUnitImpulse = glm::cross(contactPointDelta,contact.normal);
            vec3 rotationPerUnitImpulse = worldInverseInertiaTensor * torquePerUnitImpulse;
            vec3 velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse,contactPointDelta);
            float angularComponent = glm::dot(velocityPerUnitImpulse,contact.normal);
            float pointVelocityPerUnitImpuse = angularComponent + inverseMass;


            vec3 closingVelocity = getVelocityAtPointRelative(contactPointDelta);

            float deltaVelocity = -closingVelocity.y * (1+restitution);
            return deltaVelocity / pointVelocityPerUnitImpuse;
        }

        void resolveContactPosition(Contact& contact) {
            vec3 contactPointWorld = transformPoint(contact.point);
            vec3 contactPointDelta = contactPointWorld - position;
            if(contactPointWorld.y >= 0) return;
            vec3 torquePerUnitImpulse = glm::cross(contactPointDelta,contact.normal);
            vec3 rotationPerUnitImpulse = worldInverseInertiaTensor * torquePerUnitImpulse;
            vec3 velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse,contactPointDelta);
            float angularInertia = glm::dot(velocityPerUnitImpulse,contact.normal);
            float linearInertia = inverseMass;

            float totalInertia = linearInertia + angularInertia;
            float inverseInertia = 1 / totalInertia;
            float penetration = -contactPointWorld.y;
            float linearMove = penetration * linearInertia * inverseInertia;
            float angularMove = penetration * angularMove * inverseInertia;

            vec3 rotationPerMove = rotationPerUnitImpulse * (1/angularInertia);

            position += contact.normal * linearMove;
            rotate(rotationPerMove * angularMove);

        }

        void generateContacts(std::vector<Contact>& contacts) {
            generateContactWithPoint(vec3(1,1,1),contacts);
            generateContactWithPoint(vec3(-1,1,1),contacts);
            generateContactWithPoint(vec3(-1,-1,1),contacts);
            generateContactWithPoint(vec3(1,-1,1),contacts);

            generateContactWithPoint(vec3(1,1,-1),contacts);
            generateContactWithPoint(vec3(-1,1,-1),contacts);
            generateContactWithPoint(vec3(-1,-1,-1),contacts);
            generateContactWithPoint(vec3(1,-1,-1),contacts);
        }

        void generateContactWithPoint(vec3 relativePoint,std::vector<Contact>& contacts) {
            vec3 worldPoint = transformPoint(relativePoint);
            if(worldPoint.y < 0) {
                contacts.push_back(Contact(relativePoint,vec3(0,1,0)));
                Debug::drawPoint(worldPoint);
            }
        }

        float getMass() {
            if(inverseMass == 0) return std::numeric_limits<float>::infinity();
            return 1/inverseMass;
        }
        void setMass(float mass) {
            if(mass == 0) mass = 0.0001;
            inverseMass = 1/mass;
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