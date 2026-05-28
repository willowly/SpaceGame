#pragma once
#include "type-info.hpp"
#include "graphics/vulkan.hpp"


namespace TypeInfoLoader {

    inline void loadGraphics(Registry& registry) {
        
        TypeInfo* lit = registry.addTypeInfo("lit_material");
        lit->addConstProperty("texture",&LitMaterialData::texture);
        lit->addConstProperty("color",&LitMaterialData::color);
        
    }

}