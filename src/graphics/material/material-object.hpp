#pragma once
#include "graphics/vulkan.hpp"
#include "engine/object.hpp"
#include <string>

using std::string;

class MaterialObject : public Object {

    Material cachedMaterial = Material::none;

    protected:
        template<typename MaterialData>
        Material createMaterial(Vulkan* vulkan,MaterialData data) {
            return vulkan->createMaterial<MaterialData,Vertex>(shader,data,options);
        }
    
    public:
        string shader;
        PipelineOptions options;
        Material material = Material::none;
    
        virtual void loadMaterial(Vulkan* vulkan) = 0;

        virtual ~MaterialObject() {}

};