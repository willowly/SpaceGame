#pragma once
#include "actor.hpp"
#include "item/item-stack.hpp"
#include "actor/components/rigidbody.hpp"
#include "helper/random.hpp"

#include "Jolt/Physics/Collision/Shape/SphereShape.h"

#include "persistance/actor/data-item-actor.hpp"

class ItemActor : public Actor {

    

    public:
        ItemStack stack;

        Rigidbody body;


        ItemActor() {
            body.generateCollisionEvents = true;
        }

        ~ItemActor() noexcept = default;

        static std::unique_ptr<ItemActor> makeInstance(ItemStack stack,vec3 position = vec3(0),quat rotation = Random::rotation()) {
            if(stack.isEmpty()) {
                Debug::warn("trying to make an item_actor with an empty stack");
            }
            auto newActor = new ItemActor();
            std::unique_ptr<ItemActor> actor = std::unique_ptr<ItemActor>(newActor);
            actor->position = position;
            actor->rotation = rotation;
            actor->updateLastTransform();
            actor->stack = stack;
            return actor;
        }

        void spawn(World* world) override {

            Physics::initalizePhysicsGlobal();

            auto settings = body.getDefaultBodySettings(this,new JPH::SphereShape(0.3f),position,rotation);

            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            settings.mObjectLayer = Layers::ITEM;
            auto inertia = settings.GetMassProperties().mInertia;
            JPH::MassProperties massProperties;
            massProperties.mMass = 0.0001f;
            massProperties.mInertia = inertia;
            settings.mMassPropertiesOverride = massProperties;
            settings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ;

            body.spawn(world,this,settings);

            body.getBody()->SetFriction(0.5f);
            
        }

        void step(World* world,float dt) override {
            updateLastTransform();
            body.applyGravity(world,position,dt);
        }

        vec3 getVelocity() override {
            return body.getVelocity();
        }

        void setVelocity(vec3 velocity) override {
            body.setVelocity(velocity);
        }

        bool hasVelocity() override {
            return true;
        }

        vec3 getAngularVelocity() override
        {
            return body.getAngularVelocity();
        }

        void setAngularVelocity(vec3 v) override
        {
            return body.setAngularVelocity(v);
        }

        void prePhysics(World* world) override {
            body.prePhysics(world,position,rotation);
        }

        void postPhysics(World* world) override {
            body.postPhysics(world,position,rotation);
        }

        void destroy(World* world) override {
            body.destroy(world);
            Actor::destroy(world);
        }

        void addRenderables(Vulkan* vulkan,float dt,float interpolation) override {
            assert(stack.item != nullptr);
            stack.item->addRenderables(vulkan,stack,getInterpolatedPosition(interpolation),getInterpolatedRotation(interpolation));
        }

        // Saving

        string getTypeName() override {
            return "item_actor";
        }

        string getActorDataType() override
        {
            return getTypeName();
        }

        static std::unique_ptr<Actor> makeInstanceFromSave(const data_ItemActor& data,DataLoader& loader) {
            ItemStack stack;
            stack.load(data.stack,loader);
            auto actor = makeInstance(stack);
            actor->load(data,loader);

            std::cout << "LOADING ITEM ACTOR" << std::endl;

            return actor;
        }

        std::vector<std::uint8_t> createSaveBuffer() override {
            auto data = save();
            auto buf = cista::serialize(data);
            return buf;
        }

        data_ItemActor save() {
            data_ItemActor data;
            data.actor = Actor::save();
            data.body = body.save();
            data.stack = stack.save();
            return data;
        }

        void load(const data_ItemActor& data,DataLoader& loader) {
            Actor::load(data.actor);
            body.load(data.body);
            stack.load(data.stack,loader);
        }

        





};