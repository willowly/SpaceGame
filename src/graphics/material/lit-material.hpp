#pragma once

#include "glm/glm.hpp"

#include "graphics/vulkan.hpp"

#include "material-object.hpp"

struct LitMaterialData {
    LitMaterialData() {}
    LitMaterialData(TextureID texture) : texture(texture) {}
    LitMaterialData(TextureID texture,vec4 color) : texture(texture), color(color) {}
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