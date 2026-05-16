#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include "physics/structs.hpp"

#include "persistance/actor/data-actor.hpp"


using glm::vec3, glm::quat;

#ifndef WORLD
class World;
#endif

class Actor {

    protected:
        vec3 position = vec3(0);
        quat rotation = vec3(0);

        vec3 lastPosition = vec3(0);
        quat lastRotation = vec3(0);


    public:

        // for tracking prototype names
        string name;
        
        Mesh<Vertex>* model = nullptr;
        Material material = Material::none;
        float modelScale = 1;

        bool destroyed = false;

        virtual ~Actor() = default;
        Actor(const Actor& actor) = default;

        // general-purpose functions

        void setPosition(vec3 position) {
            this->position = position;
        }

        vec3 getPosition() {
            return position;
        }

        vec3 getInterpolatedPosition(float interpolation) {
            return MathHelper::lerp(lastPosition,position,interpolation);
        }

        void setRotation(quat rotation) {
            this->rotation = rotation;
        }

        quat getRotation() {
            return rotation;
        }

        quat getInterpolatedRotation(float interpolation) {
            return glm::slerp(lastRotation,rotation,interpolation);
        }


        virtual glm::mat4 getTransform() {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),position);
            matrix *= glm::toMat4(rotation);
            return matrix;
        }


        virtual glm::mat4 getInterpolatedTransform(float interpolation) {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),getInterpolatedPosition(interpolation));
            matrix *= glm::toMat4(getInterpolatedRotation(interpolation));
            return matrix;
        }

        vec3 transformPoint(vec3 point) {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),position);
            matrix *= glm::toMat4(rotation);
            glm::vec4 v4 = matrix * glm::vec4(point.x,point.y,point.z,1);
            return vec3(v4.x,v4.y,v4.z);
        }

        vec3 transformPointInterpolated(vec3 point,float interpolation) {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f),getInterpolatedPosition(interpolation));
            matrix *= glm::toMat4(getInterpolatedRotation(interpolation));
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

        void updateLastTransform() {
            lastPosition = position;
            lastRotation = rotation; 
        }

        // defines the behaviour

        virtual void spawn(World* world) {

        }

        virtual void prePhysics(World* world) {

        }

        virtual void postPhysics(World* world) {

        }

        virtual void step(World* world,float dt) {
            updateLastTransform();
        }

        virtual void collisionStart(World* world,const Collision& contact) {

        }

        virtual void addRenderables(Vulkan* vulkan,float dt,float interpolation) {
            if(model == nullptr) return; //if no model, nothing to render :)
            model->addToRender(vulkan,material,position,rotation,vec3(modelScale));
        }

        virtual void destroy(World* world) {
            destroyed = true;
        }

        // Factory Functions

        static std::unique_ptr<Actor> makeDefaultPrototype() {
            auto ptr = new Actor();
            return std::unique_ptr<Actor>(ptr);
        }

        static std::unique_ptr<Actor> makeInstance(Actor* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            return makeInstanceFromPrototype<Actor>(prototype,position,rotation);
        }

        // Saving and loading

        virtual data_ActorType getActorDataType() {
            return data_ActorType::DONT_SAVE;
        }

        virtual std::vector<std::uint8_t> createSaveBuffer() {
            auto data = save();
            auto buf = cista::serialize(data);
            return buf;
        }

        data_Actor save() {
            data_Actor data{};
            data.position.set(position);
            data.rotation.set(rotation);
            return data;
        }

        void load(const data_Actor& data) {
            position = data.position.toVec3();
            rotation = data.rotation.toQuat();
        }

        template<typename T, typename data_T>
        static std::unique_ptr<Actor> makeInstanceFromSavePrototype(data_T& data,T* prototype) {

            if(prototype == nullptr) throw std::runtime_error("prototype is null");
            auto actor = T::makeInstanceFromPrototype(prototype);
            actor->load(data);

            return actor;
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
            actor->name = prototype->name;
            actor->position = position;
            actor->rotation = rotation;
            actor->updateLastTransform();
            return actor;
        }
        

};