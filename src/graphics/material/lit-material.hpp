#pragma once

#include "glm/glm.hpp"

#include "graphics/vulkan.hpp"

#include "material-object.hpp"

struct LitMaterialData {
    TextureID texture = 0;
    vec4 color = vec4(1);
};

class LitMaterialObject : public MaterialObject {

    public:

        LitMaterialObject()  {
            shader = "lit";
        }

        LitMaterialData data;

        void loadMaterial(Vulkan* vulkan) override {
            material = createMaterial(vulkan,data);
        }

        string getTypeName() override {
            return "lit_material";
        }

    
};