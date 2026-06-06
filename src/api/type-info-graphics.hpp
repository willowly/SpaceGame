#pragma once
#include "type-info.hpp"
#include "graphics/vulkan.hpp"


namespace TypeInfoLoader {

    inline void loadGraphics(Registry& registry) {
        
        TypeInfo* lit = registry.addTypeInfo<LitMaterialData>("lit_material");
        lit->addConstProperty("texture",&LitMaterialData::texture);
        lit->addConstProperty("color",&LitMaterialData::color);

        TypeInfo* sprite = registry.addTypeInfo<Sprite>("sprite");
        sprite->addConstProperty("texture",&Sprite::texture);
        sprite->addConstProperty("rect",&Sprite::rect);
        
    }

}