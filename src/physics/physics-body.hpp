#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "engine/debug.hpp"
#include "helper/math-helper.hpp"
#include "physics/structs.hpp"
#include <vector>


using glm::vec3,glm::mat3,glm::quat;

class PhysicsBody {

    vec3 position = vec3(0);
    quat rotation = glm::identity<quat>();
    vec3 velocity = vec3(0);
    vec3 angularVelocity = vec3(0);
    float inverseMass = 1;
    bool useGravity = true;
    float restitution = 0.3f;
    float friction = 0.5;
    float staticFrictionThreshold = 0.1;

    mat3 localInverseInertiaTensor;
    mat3 worldInverseInertiaTensor;

    // shape is always cube
    glm::vec3 scale = vec3(1.0f); //this is temporary!!! only for testing a shape

    public:

        PhysicsBody(float mass) {
            vec3 d = scale * 2.0f;
            vec3 d2 = vec3(d.x*d.x,d.y*d.y,d.z*d.z);
            localInverseInertiaTensor = glm::inverse(mat3(
                vec3((d2.y*d2.z)*mass/12,0,0),
                vec3(0,(d2.x*d2.z)*mass/12,0),
                vec3(0,0,(d2.x*d2.y)*mass/12)
            ));
            transformInertiaTensor(worldInverseInertiaTensor,rotation,localInverseInertiaTensor);
        }

        void setScale(vec3 scale) {
            this->scale = scale;
        }

        vec3 getScale() {
            return scale;
        }

        void setPosition(vec3 position) {
            this->position = position;
        }

        vec3 getPosition() {
            return position;
        }

        void setRotation(quat rotation) {
            this->rotation = rotation;
        }

        quat getRotation() {
            return rotation;
        }

        void setVelocity(vec3 velocity) {
            this->velocity = velocity;
        }

        vec3 getVelocity() {
            return velocity;
        }

        void setAngularVelocity(vec3 angularVelocity) {
            this->angularVelocity = angularVelocity;
        }

        vec3 getAngularVelocity() {
            return angularVelocity;
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

        float getMass() {
            if(inverseMass == 0) return std::numeric_limits<float>::infinity();
            return 1/inverseMass;
        }
        void setMass(float mass) {
            if(mass == 0) mass = 0.0001;
            inverseMass = 1/mass;
        }

        //this is from actor, maybe at some point we can factor it out but rn no need

        glm::mat4 getTransform() {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),position);
            matrix *= glm::toMat4(rotation);
            return matrix;
        }

        vec3 transformPoint(vec3 point) {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),position);
            matrix *= glm::toMat4(rotation);
            glm::vec4 v4 = matrix * glm::vec4(point.x,point.y,point.z,1);
            return vec3(v4.x,v4.y,v4.z);
        }

        vec3 inverseTransformPoint(vec3 point) {
            glm::mat4 matrix = glm::toMat4(glm::inverse(rotation));
            matrix = glm::translate(matrix,-position);
            glm::vec4 v4 = matrix * glm::vec4(point.x,point.y,point.z,1);
            return vec3(v4.x,v4.y,v4.z);
        }

        vec3 transformDirection(vec3 direction) {
            glm::vec4 v4 = glm::toMat4(rotation) * glm::vec4(direction.x,direction.y,direction.z,1);
            return vec3(v4.x,v4.y,v4.z);
        }

        vec3 inverseTransformDirection(vec3 direction) {
            glm::vec4 v4 = glm::toMat4(glm::inverse(rotation)) * glm::vec4(direction.x,direction.y,direction.z,1);
            return vec3(v4.x,v4.y,v4.z);
        }

        void rotate(vec3 eulerAngles) {
            rotation *= glm::angleAxis(glm::radians(eulerAngles.x),vec3(1,0,0));
            rotation *= glm::angleAxis(glm::radians(eulerAngles.y),vec3(0,1,0));
            rotation *= glm::angleAxis(glm::radians(eulerAngles.z),vec3(0,0,1));
        }

        void integrate(float dt) {
            //if(useGravity) velocity += world->getGravityVector(position) * dt; gravity can be applied in the main loop
            position += velocity * dt;
            
            //velocity = velocity * 0.99f;
            
            rotation += dt * 0.5f * glm::quat(0,angularVelocity.x,angularVelocity.y,angularVelocity.z) * rotation;
            rotation = glm::normalize(rotation);
            transformInertiaTensor(worldInverseInertiaTensor,rotation,localInverseInertiaTensor);
        }

        //end from actor

        static void transformInertiaTensor(glm::mat3 &iitWorld, const glm::quat &q, const glm::mat3 &iitBody)
        {

            glm::mat4 qMat = glm::toMat4(q);
            const float* rotMatData = glm::value_ptr(qMat);
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

        // These should be moved somewhere else

        void collideWithTriangle(vec3 a,vec3 b,vec3 c) {
            std::vector<Contact> contacts;
            generateContacts(contacts,a,b,c);
            if(contacts.size() == 0) return;
            
            for (int i = 0; i <= 10; i++)
            {
                vec3 greatestContactForce = getContactForce(contacts[0]);
                Contact worst = contacts[0];
                for (Contact& contact : contacts)
                {
                    vec3 force = getContactForce(contact);
                    if(glm::length(force) > glm::length(greatestContactForce)) {
                        worst = contact;
                        greatestContactForce = force;
                    }
                }
                if(glm::length(greatestContactForce) > 0.0001) {
                    Debug::drawRay(transformPoint(worst.point),greatestContactForce);
                    applyForce(greatestContactForce,transformPoint(worst.point));
                }
            }
 
            for (int i = 0; i <= 10; i++)
            {
                Contact worst = contacts[0];
                for (Contact& contact : contacts)
                {
                    if(contact.penetration > worst.penetration) { //penetration
                        worst = contact;
                    }
                }
                if(worst.penetration <= 0) continue;
                auto movement = resolveContactPosition(worst);

                // update the penetration values now that we've moved
                for (Contact& contact : contacts)
                {
                    quat rotation = glm::quat(glm::radians(vec3(movement.second)));
                    vec3 linear = ((contact.point * rotation) - contact.point) + movement.first;
                    contact.penetration -= glm::dot(linear,contact.normal);
                }
            }
            //position += deltaPosition;
            
        }

        vec3 getContactForce(Contact& contact) {
            vec3 contactPointWorld = transformPoint(contact.point);
            vec3 contactPointDelta = contactPointWorld - position;

            mat3 impulseToTorque = mat3(
                vec3(0,                     -contactPointDelta.z,   contactPointDelta.y),
                vec3(contactPointDelta.z,   0,                      -contactPointDelta.x),
                vec3(-contactPointDelta.y,  contactPointDelta.x,    0)
            );

            mat3 deltaVelWorld = impulseToTorque;
            deltaVelWorld *= worldInverseInertiaTensor;
            deltaVelWorld *= impulseToTorque;
            deltaVelWorld *= -1;
            
            quat worldToContact = glm::rotation(vec3(1.0f,0,0),contact.normal); // get rotation
            mat3 contactToWorldMatrix = glm::mat3(worldToContact*vec3(1.0f,0,0),worldToContact*vec3(0.0,1.0f,0.0),worldToContact*vec3(0.0f,0.0f,1.0f));
            
            mat3 deltaVelocity = glm::transpose(contactToWorldMatrix) * deltaVelWorld * contactToWorldMatrix;

            float* deltaVelocity_valuePtr = glm::value_ptr(deltaVelocity);

            deltaVelocity_valuePtr[0] += inverseMass;
            deltaVelocity_valuePtr[4] += inverseMass;
            deltaVelocity_valuePtr[8] += inverseMass;

            mat3 impulseMatrix = glm::inverse(deltaVelocity);


            vec3 contactVelocity = worldToContact * getVelocityAtPointRelative(contactPointDelta); //transform into contact space?

            //desiredDeltaVelocity = closingVelocity

            if(contactVelocity.y > 0) return vec3(0.0f); //if its moving away, we dont need to apply any velocity

            vec3 velKill = vec3(
                -contactVelocity.x * (1+restitution),
                -contactVelocity.y,
                -contactVelocity.z
            );

            vec3 impulseContact = velKill * impulseMatrix;

            
            float planarImpulse = glm::length(vec2(impulseContact.x,impulseContact.z));
            if(planarImpulse > friction * impulseContact.y) {
                //scale the vector to be exactly the right size
                impulseContact.y /= planarImpulse;
                impulseContact.z /= planarImpulse;

                impulseContact.x = //why are we modifying the y impulse??
                    deltaVelocity_valuePtr[0] +
                    deltaVelocity_valuePtr[1] * friction * impulseContact.x + 
                    deltaVelocity_valuePtr[2] * friction * impulseContact.z;

                impulseContact.x = velKill.x / impulseContact.x;
                impulseContact.y *= friction * impulseContact.x;
                impulseContact.z *= friction * impulseContact.x;

            }

            //impulse.y = std::max(impulse.y,0.0f);
            return glm::inverse(worldToContact) * impulseContact;
        }

        std::pair<vec3,vec3> resolveContactPosition(Contact& contact) {
            vec3 contactPointWorld = transformPoint(contact.point);
            vec3 contactPointDelta = contactPointWorld - position;
            //if(contactPointWorld.y >= 0) return;
            vec3 torquePerUnitImpulse = glm::cross(contactPointDelta,contact.normal);
            vec3 rotationPerUnitImpulse = worldInverseInertiaTensor * torquePerUnitImpulse;
            vec3 velocityPerUnitImpulse = glm::cross(rotationPerUnitImpulse,contactPointDelta);
            float angularInertia = glm::dot(velocityPerUnitImpulse,contact.normal);
            float linearInertia = inverseMass;

            float totalInertia = linearInertia + angularInertia;
            float inverseInertia = 1 / totalInertia;
            float penetration = contact.penetration;
            float linearMove = penetration * linearInertia * inverseInertia;
            float angularMove = penetration * angularInertia * inverseInertia;

            vec3 rotationPerMove = rotationPerUnitImpulse * (1/angularInertia);

            position += contact.normal * linearMove;
            rotate(rotationPerMove * angularMove);
            
            //position += contact.normal * penetration; //fuck the disc

            return std::pair(contact.normal * linearMove,rotationPerMove * angularMove);
        }

        void generateContacts(std::vector<Contact>& contacts,vec3 a,vec3 b,vec3 c) {
            generateContactWithPoint(vec3(1,1,1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(-1,1,1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(-1,-1,1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(1,-1,1)*scale,a,b,c,contacts);

            generateContactWithPoint(vec3(1,1,-1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(-1,1,-1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(-1,-1,-1)*scale,a,b,c,contacts);
            generateContactWithPoint(vec3(1,-1,-1)*scale,a,b,c,contacts);
        }

        void generateContactWithPoint(vec3 relativePoint,vec3 a,vec3 b,vec3 c,std::vector<Contact>& contacts) {
            vec3 worldPoint = transformPoint(relativePoint);
            vec3 normal = MathHelper::normalFromPlanePoints(a,b,c);
            vec3 toPlane = a - worldPoint;
            //Debug::drawRay(a,normal);

            if(glm::dot(normal,toPlane) > 0) {
                Debug::drawRay(worldPoint,glm::dot(toPlane,normal)*normal);
                contacts.push_back(Contact(relativePoint,normal,glm::dot(toPlane,normal)));
                Debug::drawPoint(worldPoint);
            }

            // if(worldPoint.y < 0) {
            //     contacts.push_back(Contact(relativePoint,vec3(0,1,0),-worldPoint.y));
            //     Debug::drawPoint(worldPoint);
            // }
        }

};