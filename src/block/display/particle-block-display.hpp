
#include "block-display.hpp"
#include "actor/components/particle-effect.hpp"

class ParticleBlockDisplay : public BlockDisplay {

    public:
        ParticleEffect effect;
        static const int EMISSION_VAR = 0;

        ParticleBlockDisplay(ParticleEffectSettings* settings,ivec3 position,quat rotation) : effect(settings), BlockDisplay(position,rotation) {
            storage.setFloat(EMISSION_VAR,1);
            this->effect.spawn(position,rotation);
        }

        void addRenderables(Vulkan* vulkan,Construction* construction,float dt,float interpolation) override {
            effect.addRenderables(vulkan,dt);
            float emission = storage.getFloat(EMISSION_VAR);
            effect.emission = emission;
            effect.step(construction->transformPointInterpolated(position,interpolation),construction->getInterpolatedRotation(interpolation) * rotation,construction->getVelocity(),dt);
        }

};