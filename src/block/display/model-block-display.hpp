
#include "graphics/basic-model.hpp"
#include "block-display.hpp"

class ModelBlockDisplay : public BlockDisplay {

    public:
        BasicModel model;
        static const int SPEED_VAR = 0;

        ModelBlockDisplay(BasicModel model,float speed,ivec3 position,quat rotation) : model(model), BlockDisplay(position,rotation) {
            storage.setFloat(SPEED_VAR,speed);
        }

        void addRenderables(Vulkan* vulkan,Construction* construction,float dt,float interpolation) override {
            auto speed = storage.getFloat(SPEED_VAR);
            model.rotation *= quat(vec3(0,0,speed * dt));
            model.addRenderables(vulkan,construction->transformPointInterpolated(position,interpolation),construction->getInterpolatedRotation(interpolation) * rotation);
        }

};