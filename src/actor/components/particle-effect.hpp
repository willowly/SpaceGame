#pragma once

#include "graphics/vulkan.hpp"
#include "engine/debug.hpp"

#include <vector>
#include <queue>

#include <glm/gtc/random.hpp>

using glm::vec3;



class ParticleEffect {

    private:
        struct Particle {
            vec3 pos = {};
            vec3 velocity = {};
            quat rotation = {};
            vec3 angularVelocity = {};
            float age = -1; // -1 means they aren't spawned
            float lifeTime;
        };

        std::vector<Particle> particles;
        size_t nextIndex = 0;
        float spawnTimer = 0;
        int particlesAlive = 0;

    public:

        struct Range {
            float start = 0.0f;
            float end = 0.0f;
            Range() {};
            Range(float constant) : start(constant), end(constant) {}
            Range(float start,float end) : start(start), end(end) {}

            float sample(float t) {
                return MathHelper::lerp(start,end,t);
            }

            float sampleRandom() {
                return MathHelper::lerp(start,end,glm::linearRand(0.0f,1.0f));
            }
        };


        Mesh<Vertex>* mesh = nullptr;
        Material material = Material::none;
        int spawnRate = 1;
        int initialSpawnCount = 0;
        Range initialVelocity = 1.0f;
        Range lifeTime = 1;
        Range particleSize = 0.2f;
        Range initialAngularVelocity;

       

        struct SphereShape {

            // sphere
            float radius;

            void setPositionAndDirection(Particle& particle,vec3 position,quat rotation) {

                vec3 spherePos = glm::sphericalRand(1.0f);
                particle.pos = (glm::linearRand(0.0f,1.0f) * spherePos) + position;
                particle.velocity = spherePos;
            }


        } emitterShape;

        void spawn(vec3 position,quat rotation) {

            int amount = ceil(spawnRate * lifeTime.end) + 1 + initialSpawnCount;

            particles.clear();

            particles.reserve(amount);

            for (size_t i = 0; i < amount; i++)
            {
                particles.push_back(Particle());
            }

            for (size_t i = 0; i < initialSpawnCount; i++)
            {
                spawnParticle(position,rotation);
            }
            
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            for (size_t i = 0; i < particles.size(); i++)
            {
                particleRender(particles[i],vulkan,dt);
            }
        }

        void step(vec3 position,quat rotation,float dt) {
            for (size_t i = 0; i < particles.size(); i++)
            {
                particleStep(particles[i],dt);
            }
            while(spawnTimer <= 0) {
                spawnParticle(position,rotation);
                spawnTimer += 1.0f;
            }
            spawnTimer -= dt * spawnRate;
            
        }

        void spawnParticle(vec3 position,quat rotation) {
            auto& newParticle = particles[nextIndex];

            if(newParticle.age > 0) {
                Debug::warn("particle count not high enough!");
            }

            //vec3 spawnPosRelative = 
            emitterShape.setPositionAndDirection(newParticle,position,rotation);
            newParticle.age = 0;
            newParticle.velocity *= initialVelocity.sampleRandom();

            newParticle.lifeTime = lifeTime.sampleRandom();

            newParticle.rotation = glm::quat(glm::linearRand(vec3(-360),vec3(360)));
            newParticle.angularVelocity = glm::linearRand(vec3(-360),vec3(360)) * initialAngularVelocity.sampleRandom();
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
            matrix = glm::scale(matrix,vec3(particleSize.sample(t)));
            RenderingSettings settings;
            //settings.faceCamera = true;
            vulkan->addMesh(mesh->meshBuffer,material,settings,matrix);
        }

        int getParticlesAlive() {
            return particlesAlive;
        }
};