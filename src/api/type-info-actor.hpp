#pragma once
#include "type-info.hpp"
#include "actor/actors-all.hpp"


namespace TypeInfoLoader {

    inline void loadActor(Registry& registry) {

        auto actor = registry.addTypeInfo<Actor>("actor");

        auto character = registry.addTypeInfo<Character>("character");
        character->constructorFunction = [&]() {return Character::makeDefaultPrototype();};
        character->addConstProperty("move_speed",&Character::moveSpeed);
        character->addConstProperty("look_sensitivity",&Character::lookSensitivity);
        character->addConstProperty("height",&Character::height);
        character->addConstProperty("radius",&Character::radius);
        character->addConstProperty("ground_acceleration",&Character::groundAcceleration);
        character->addConstProperty("ground_decelleration",&Character::groundDecelleration);
        character->addConstProperty("air_acceleration",&Character::airAcceleration);
        character->addConstProperty("jump_force",&Character::jumpForce);
        character->addConstProperty("craft_speed",&Character::craftSpeed);
        character->addConstProperty("rotation_speed",&Character::rotationSpeed);
        character->addConstProperty("rotation_acceleration",&Character::rotationAcceleration);
        character->addConstProperty("item_drop_distance",&Character::itemDropDistance);
        character->addConstProperty("third_person_camera_offset",&Character::thirdPersonCameraOffset);
        character->addConstProperty("third_person_camera_rot",&Character::thirdPersonCameraRot);
        character->addConstProperty("input_buffer",&Character::inputBuffer);
        character->addConstProperty("coyote_time",&Character::coyoteTime);
        character->addConstProperty("camera_clear_radius",&Character::cameraClearRadius);
        character->addConstProperty("inventory_disabled",&Character::inventoryDisabled);
        character->addConstProperty("model",&Character::model);
        character->addConstProperty("material",&Character::material);
        character->addConstProperty("model_scale",&Character::modelScale);
        character->addConstProperty("widget",&Character::widget);
        character->setParent(actor);


        auto terrainType = registry.addTypeInfo<TerrainType>("terrain_type");
        terrainType->addConstProperty("item",&TerrainType::item);
        terrainType->addConstProperty("texture",&TerrainType::texture);

        auto terrainSettings = registry.addTypeInfo<TerrainSettings>("terrain_settings");
        terrainSettings->addConstProperty("generation_settings",&TerrainSettings::generationSettings);
        terrainSettings->addConstProperty("gravity",&TerrainSettings::gravity);
        terrainSettings->addConstProperty("chunk_size",&TerrainSettings::chunkSize);
        terrainSettings->addConstProperty("base_cell_size",&TerrainSettings::baseCellSize);
        terrainSettings->addConstProperty("lod_distance",&TerrainSettings::LODdistance);
        terrainSettings->addConstProperty("lod_distance_factor",&TerrainSettings::LODdistanceFactor);

        auto generationSettings = registry.addTypeInfo<GenerationSettings>("generation_settings");

        generationSettings->addConstProperty("noise_scale",&GenerationSettings::noiseScale);
        generationSettings->addConstProperty("radius",&GenerationSettings::radius);
        generationSettings->addConstProperty("noise_factor",&GenerationSettings::noiseFactor);
        generationSettings->addConstProperty("noise_octaves",&GenerationSettings::noiseOctaves);
        generationSettings->addConstProperty("noise_gain",&GenerationSettings::noiseGain);
        generationSettings->addConstProperty("noise_lacunarity",&GenerationSettings::noiseLacunarity);
        generationSettings->addConstProperty("stone_type",&GenerationSettings::stoneType); 
        generationSettings->addConstProperty("ore_type",&GenerationSettings::oreType); 
        
       


    }

}