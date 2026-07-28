#pragma once

#include "graphics/vulkan.hpp"
#include "engine/debug.hpp"

#include <vector>
#include <queue>

#include <glm/gtc/random.hpp>

using glm::vec3;

struct FloatRange {
    float start = 0.0f;
    float end = 0.0f;
    FloatRange() {};
    FloatRange(float constant) : start(constant), end(constant) {}
    FloatRange(float start,float end) : start(start), end(end) {}

    float sample(float t) {
        return MathHelper::lerp(start,end,t);
    }

    float sampleRandom() {
        return MathHelper::lerp(start,end,glm::linearRand(0.0f,1.0f));
    }
};

enum class EmitterShape {
    Sphere,
    Cone
};

struct ParticleEffectSettings {

    
    Mesh<Vertex>* mesh = nullptr;
    MaterialObject* material = nullptr;
    float spawnRate = 1;
    int initialSpawnCount = 0;
    FloatRange initialVelocity = 1.0f;
    bool inheritVelocity = false;
    FloatRange lifeTime = 1;
    FloatRange particleSize = 0.2f;
    FloatRange initialAngularVelocity;
    bool faceCamera;

    vec3 emitterOffset = {};
    EmitterShape emitterShape = {};
    float emitterRadius = 0;
    
};


class ParticleEffect {

    private:
        struct Particle {
            vec3 pos = {};
            vec3 velocity = {};
            quat rotation = {};
            vec3 angularVelocity = {};
            float age = -1; // -1 means they aren't spawned
            float lifeTime = 1;
        };

        std::vector<Particle> particles;
        size_t nextIndex = 0;
        float spawnTimer = 0;
        int particlesAlive = 0;

        void spawnParticle(vec3 position,quat rotation,vec3 velocity = {}) {
            if(particles.size() == 0) return;
            auto& newParticle = particles[nextIndex];

            if(newParticle.age > 0) {
                Debug::warn("particle count not high enough!");
            }

            //vec3 spawnPosRelative = 
            switch(settings->emitterShape) {
                case EmitterShape::Sphere:
                    
                    vec3 spherePos = glm::sphericalRand(1.0f);
                    newParticle.pos = (glm::linearRand(0.0f,settings->emitterRadius) * spherePos) + position;
                    newParticle.velocity = spherePos;
                    break;
                case EmitterShape::Cone:
                    vec2 circlePos = glm::circularRand(1.0f);
                    circlePos *= glm::linearRand(0.0f,settings->emitterRadius);
                    newParticle.pos = (rotation * vec3(circlePos.x,circlePos.y,0)) + position;
                    newParticle.velocity = rotation * vec3(0,0,1);
                    break;
            }
            newParticle.pos += rotation * settings->emitterOffset;
            newParticle.age = 0;
            newParticle.velocity *= settings->initialVelocity.sampleRandom();
            if(settings->inheritVelocity) {
                newParticle.velocity += velocity;
            }

            newParticle.lifeTime = settings->lifeTime.sampleRandom();

            newParticle.rotation = glm::quat(glm::linearRand(vec3(-360),vec3(360)));
            newParticle.angularVelocity = glm::linearRand(vec3(-360),vec3(360)) * settings->initialAngularVelocity.sampleRandom();
            nextIndex++;
            if(nextIndex >= particles.size()) {
                nextIndex = 0;
            }
            particlesAlive++;
        }

        void particleStep(Particle& particle,float dt) {
            if(particle.age < 0) return;

            particle.age += dt;
            particle.pos += particle.velocity * dt;
            if(particle.age > particle.lifeTime) {
                particle.age = -1; // destroy;
                particlesAlive--;
            }
        }

        void particleRender(Particle& particle,Vulkan* vulkan,float dt) {
            if(particle.age < 0) return;

            float t = particle.age/particle.lifeTime;

            auto matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix,particle.pos);
            matrix *= glm::toMat4(particle.rotation);
            matrix = glm::scale(matrix,vec3(settings->particleSize.sample(t)));
            RenderingSettings renderSettings;
            renderSettings.faceCamera = settings->faceCamera;
            vulkan->addMesh(settings->mesh->meshBuffer,settings->material->material,renderSettings,matrix);
        }

    public:

        ParticleEffectSettings* settings = nullptr;
        float emission = 1;

        ParticleEffect(ParticleEffectSettings* settings) : settings(settings) {
            
        }

        void spawn(vec3 position,quat rotation,vec3 velocity = {}) {

            if(settings == nullptr) {
                Debug::warn("particle effect spawned with no settings");
                return;
            }

            int amount = static_cast<int>(ceil(settings->spawnRate * settings->lifeTime.end)) + 1 + settings->initialSpawnCount;

            particles.clear();

            particles.reserve(amount);

            for (size_t i = 0; i < amount; i++)
            {
                particles.push_back(Particle());
            }

            for (size_t i = 0; i < settings->initialSpawnCount; i++)
            {
                spawnParticle(position,rotation,velocity);
            }
            
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            if(settings == nullptr) {
                return;
            }
            if(settings->material == nullptr) {
                return;
            }
            for (size_t i = 0; i < particles.size(); i++)
            {
                particleRender(particles[i],vulkan,dt);
            }
        }

        void step(vec3 position,quat rotation,vec3 velocity,float dt) {
            if(settings == nullptr) {
                return;
            }
            for (size_t i = 0; i < particles.size(); i++)
            {
                particleStep(particles[i],dt);
            }
            if(emission > 0) {
                while(spawnTimer <= 0) {
                    spawnParticle(position,rotation,velocity);
                    spawnTimer += 1.0f;
                }
                spawnTimer -= dt * settings->spawnRate * emission;
            }
            
        }

        void step(vec3 position,quat rotation,float dt) {
            step(position,rotation,vec3(0.0f),dt);
        }

        int getParticlesAlive() {
            return particlesAlive;
        }
};