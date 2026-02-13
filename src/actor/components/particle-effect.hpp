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
            float lifeTime = -1; // -1 means they aren't spawned
        };

        std::vector<Particle> particles;
        int nextIndex = 0;
        float spawnTimer = 0;

    public:
        Mesh<Vertex>* mesh = nullptr;
        Material material = Material::none;
        int spawnRate = 1;
        float initialVelocity = 1;
        float lifeTime = 1;
        float particleSize = 0.2f;

        void spawn() {

            int amount = spawnRate * lifeTime + 1;

            particles.clear();

            particles.reserve(amount);

            for (size_t i = 0; i < amount; i++)
            {
                particles.push_back(Particle());
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
                spawnTimer += 1.0f / spawnRate;
            }
            spawnTimer -= dt;
            
        }

        void spawnParticle(vec3 position,quat rotation) {
            auto& newParticle = particles[nextIndex];

            if(newParticle.lifeTime > 0) {
                Debug::warn("particle count not high enough!");
            }
            newParticle.pos = position;
            newParticle.lifeTime = 0;
            newParticle.velocity = glm::normalize(glm::sphericalRand(1.0f)); //shape
            newParticle.velocity *= initialVelocity;
            nextIndex++;
            if(nextIndex >= particles.size()) {
                nextIndex = 0;
            }
        }

        void particleStep(Particle& particle,float dt) {
            if(particle.lifeTime < 0) return;

            particle.lifeTime += dt;
            particle.pos += particle.velocity * dt;
            if(particle.lifeTime > lifeTime) {
                particle.lifeTime = -1; // destroy;
            }
        }

        void particleRender(Particle& particle,Vulkan* vulkan,float dt) {
            if(particle.lifeTime < 0) return;

            auto matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix,particle.pos);
            matrix = glm::scale(matrix,vec3(particleSize));
            mesh->addToRender(vulkan,material,matrix);
        }
};