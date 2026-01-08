#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "graphics/mesh.hpp"
#include "physics/intersections.hpp"
#include "graphics/vulkan.hpp"


using glm::vec3, glm::quat;

#ifndef WORLD
class World;
#endif

class Actor {

    public:
        vec3 position = vec3(0);
        quat rotation = vec3(0);
        Mesh<Vertex>* model = nullptr;
        Material material = Material::none;
        float modelScale = 1;

        bool destroyed = false;

        virtual ~Actor() {}


        virtual glm::mat4 getTransform() {
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

        virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {
            return std::nullopt;
        }

        virtual void step(World* world,float dt) {
            
        }

        virtual void collideBasic(Actor* actor,float height,float radius) {

        }

        virtual void addRenderables(Vulkan* vulkan,float dt) {
            if(model == nullptr) return; //if no model, nothing to render :)
            model->addToRender(vulkan,material,position,rotation,vec3(modelScale));
        }

        //for now to get the player to move differently than the physics sim :)
        virtual bool playerStep() {
            return false;
        }

        virtual void destroy() {
            destroyed = true;
        }

        static std::unique_ptr<Actor> makeDefaultPrototype() {
            auto ptr = new Actor();
            return std::unique_ptr<Actor>(ptr);
        }

        static std::unique_ptr<Actor> makeInstance(Actor* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            return makeInstanceFromPrototype<Actor>(prototype,position,rotation);
        }

    protected:

        Actor() {

        }

        template<typename T>
        static std::unique_ptr<T> makeInstanceFromPrototype(T* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            if(prototype == nullptr) {
                Debug::warn("tried to make an actor with a null prototype");
                return nullptr;
            }
            auto newActor = new T(*prototype);
            std::unique_ptr<T> actor = std::unique_ptr<T>(newActor);
            actor->position = position;
            actor->rotation = rotation;
            return actor;
        }
        

};


// 
struct ActorUserData {
    Actor* actor;
};