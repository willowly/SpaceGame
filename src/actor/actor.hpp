#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "graphics/model.hpp"
#include "graphics/material.hpp"

#include <reactphysics3d/reactphysics3d.h>

using glm::vec3, glm::quat;

#ifndef WORLD
class World;
#endif

class Actor {

    public:
        vec3 position = vec3(0);
        vec3 velocity = vec3(0);
        quat rotation = vec3(0);
        Model* model = nullptr;
        Material* material = nullptr;
        bool useGravity = false; //doesn't move
        float modelScale = 1;

        Actor() {

        }

        Actor(Model* model,Material* material) : model(model), material(material) {

        }

        glm::mat4 transform() {
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

        virtual void step(float dt,World* world) {
            position += velocity * dt;
        }

        virtual void render(Camera& camera,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            if(material == nullptr) {
                std::cout << "null material" << std::endl;
            }
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix,position);
            matrix = matrix * glm::toMat4(rotation);
            matrix = glm::scale(matrix,vec3(modelScale));
            model->render(matrix,camera,*material);
        }

        //for now to get the player to move differently than the physics sim :)
        virtual bool playerStep() {
            return false;
        }

        virtual void addToPhysicsWorld(rp3d::PhysicsWorld* world,rp3d::PhysicsCommon* common) {

        }

        virtual void updatePhysicsRepresentation() {

        }

        virtual void updateFromPhysicsRepresentation() {
            
        }

};


// 
struct ActorUserData {
    Actor* actor;
};