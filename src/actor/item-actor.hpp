#pragma once
#include "actor.hpp"
#include "item/item-stack.hpp"
#include "actor/components/rigidbody.hpp"
#include "actor/character.hpp"
#include "helper/random.hpp"

#include "Jolt/Physics/Collision/Shape/SphereShape.h"

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
                Debug::warn("tried to make an item_actor with an empty stack");
                return nullptr;
            }
            auto newActor = new ItemActor();
            std::unique_ptr<ItemActor> actor = std::unique_ptr<ItemActor>(newActor);
            actor->position = position;
            actor->rotation = rotation;
            actor->stack = stack;
            return actor;
        }

        void spawn(World* world) override {

            auto settings = body.getDefaultBodySettings(this,new JPH::SphereShape(0.3f),position,rotation);

            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            settings.mObjectLayer = Layers::ITEM;
            auto inertia = settings.GetMassProperties().mInertia;
            JPH::MassProperties massProperties;
            massProperties.mMass = 0.0001f;
            massProperties.mInertia = inertia;
            settings.mMassPropertiesOverride = massProperties;

            body.spawn(world,this,settings);

            body.getBody()->SetFriction(0.5f);
            
        }

        

        virtual void prePhysics(World* world) override {
            body.prePhysics(world,position,rotation);
        }

        virtual void postPhysics(World* world) override {
            body.postPhysics(world,position,rotation);
        }

        void destroy(World* world) override {
            body.destroy(world);
            Actor::destroy(world);
        }

        void addRenderables(Vulkan* vulkan,float dt) override {
            stack.item->addRenderables(vulkan,stack,position,rotation);
        }

        





};