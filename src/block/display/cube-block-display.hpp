
#include "graphics/basic-model.hpp"
#include "block-display.hpp"

class BlockCubeDisplay : public BlockDisplay {

    public:
        BasicModel model;
        static const int SPEED_VAR = 0;

        BlockCubeDisplay(BasicModel model,float speed,ivec3 position,quat rotation) : model(model), BlockDisplay(position,rotation) {
            storage.setFloat(SPEED_VAR,speed);
        }

        void addRenderables(Vulkan* vulkan,Construction* construction,float dt,float interpolation) override {
            auto speed = storage.getFloat(SPEED_VAR);
            model.rotation *= quat(vec3(0,speed * dt,0));
            model.addRenderables(vulkan,construction->transformPointInterpolated(position,interpolation),construction->getInterpolatedRotation(interpolation) * rotation);
        }

};