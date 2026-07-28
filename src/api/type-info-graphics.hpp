#pragma once
#include "type-info.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/material/lit-material.hpp"
#include "graphics/basic-model.hpp"
#include "actor/components/particle-effect.hpp"


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

        TypeInfo* basicModel = registry.addTypeInfo<BasicModel>("basic_model");
        basicModel->addConstProperty("mesh",&BasicModel::mesh);
        basicModel->addConstProperty("material",&BasicModel::material);
        basicModel->addConstProperty("offset",&BasicModel::offset);
        basicModel->addConstProperty("rotation",&BasicModel::rotation);
        basicModel->addConstProperty("scale",&BasicModel::scale);

        TypeInfo* particleEffectSettings = registry.addTypeInfo<ParticleEffectSettings>("particle_effect_settings");
        
        particleEffectSettings->addConstProperty("mesh",&ParticleEffectSettings::mesh);
        particleEffectSettings->addConstProperty("material",&ParticleEffectSettings::material);
        particleEffectSettings->addConstProperty("spawn_rate",&ParticleEffectSettings::spawnRate);
        particleEffectSettings->addConstProperty("initial_spawn_count",&ParticleEffectSettings::initialSpawnCount);
        particleEffectSettings->addConstProperty("initial_velocity",&ParticleEffectSettings::initialVelocity);
        particleEffectSettings->addConstProperty("inherit_velocity",&ParticleEffectSettings::inheritVelocity);
        particleEffectSettings->addConstProperty("life_time",&ParticleEffectSettings::lifeTime); 
        particleEffectSettings->addConstProperty("particle_size",&ParticleEffectSettings::particleSize);
        particleEffectSettings->addConstProperty("initial_angular_velocity",&ParticleEffectSettings::initialAngularVelocity);
        particleEffectSettings->addConstProperty("emitter_shape",&ParticleEffectSettings::emitterShape);
        particleEffectSettings->addConstProperty("emitter_offset",&ParticleEffectSettings::emitterOffset);
        particleEffectSettings->addConstProperty("emitter_radius",&ParticleEffectSettings::emitterRadius);

        TypeInfo* floatRange = registry.addTypeInfo<FloatRange>("float_range");
        floatRange->addConstProperty("start",&FloatRange::start);
        floatRange->addConstProperty("end",&FloatRange::end);
        
    }

}