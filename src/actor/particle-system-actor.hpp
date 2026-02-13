#pragma once
#include "actor.hpp"
#include "components/particle-effect.hpp"


class ParticleEffectActor : public Actor {

    

    public:
        ParticleEffect effect;

        virtual ~ParticleEffectActor() noexcept = default;

        static std::unique_ptr<ParticleEffectActor> makeDefaultPrototype() {
            auto ptr = new ParticleEffectActor();
            return std::unique_ptr<ParticleEffectActor>(ptr);
        }

        static std::unique_ptr<ParticleEffectActor> makeInstance(ParticleEffectActor* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            return makeInstanceFromPrototype<ParticleEffectActor>(prototype,position,rotation);
        }

        void spawn(World* world) override {

            effect.spawn();
            
        }

        void addRenderables(Vulkan* vulkan,float dt) override {
            effect.addRenderables(vulkan,dt);
        }

        void step(World* world,float dt) {
            effect.step(getPosition(),getRotation(),dt);
            
        }

        





};