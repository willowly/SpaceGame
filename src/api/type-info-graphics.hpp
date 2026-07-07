#pragma once
#include "type-info.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/material/lit-material.hpp"


namespace TypeInfoLoader {

    inline void loadGraphics(Registry& registry) {
        
        TypeInfo* materialObject = registry.addTypeInfo<MaterialObject>("material_object");
        materialObject->addConstProperty("shader",&MaterialObject::shader);
        materialObject->setFolderName("materials");
        
        TypeInfo* lit = registry.addTypeInfo<LitMaterialObject>("lit_material");
        lit->constructorFunction = []() {
            return std::make_unique<LitMaterialObject>();
        };
        lit->setParent(materialObject);
        lit->addConstProperty("data",&LitMaterialObject::data);

        TypeInfo* litData = registry.addTypeInfo<LitMaterialData>("lit_material_data");
        litData->addConstProperty("texture",&LitMaterialData::texture);
        litData->addConstProperty("color",&LitMaterialData::color);
        
        TypeInfo* sprite = registry.addTypeInfo<Sprite>("sprite");
        sprite->addConstProperty("texture",&Sprite::texture);
        sprite->addConstProperty("rect",&Sprite::rect);

        TypeInfo* mesh = registry.addTypeInfo<Mesh<Vertex>>("mesh");
        
    }

}