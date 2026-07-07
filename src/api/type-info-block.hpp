#pragma once
#include "type-info.hpp"
#include "block/blocks-all.hpp"


namespace TypeInfoLoader {

    inline void loadBlock(Registry& registry) {
        
        TypeInfo* block = registry.addTypeInfo<Block>("block");
        block->addConstProperty("default_drop",&Block::defaultDrop); 
        block->addConstProperty("solid",&Block::solid);

        TypeInfo* thruster = registry.addTypeInfo<ThrusterBlock>("thruster");
        thruster->constructorFunction = [&]() {return std::make_unique<ThrusterBlock>();};
        thruster->addConstProperty("force",&ThrusterBlock::force);
        thruster->addConstProperty("side_force",&ThrusterBlock::sideForce);
        thruster->addConstProperty("mesh",&ThrusterBlock::mesh);
        thruster->addConstProperty("texture",&ThrusterBlock::texture);
        thruster->addConstProperty("cube_model",&ThrusterBlock::cubeModel);
        thruster->setParent(block);

        TypeInfo* cockpit = registry.addTypeInfo<CockpitBlock>("cockpit");
        cockpit->constructorFunction = [&]() {return std::make_unique<CockpitBlock>();};
        cockpit->addConstProperty("mesh",&CockpitBlock::mesh);
        cockpit->addConstProperty("texture",&CockpitBlock::texture);
        cockpit->setParent(block);

        TypeInfo* furnace = registry.addTypeInfo<FurnaceBlock>("furnace");
        furnace->constructorFunction = [&]() {return std::make_unique<FurnaceBlock>();};
        furnace->addConstProperty("fuel_max",&FurnaceBlock::fuelMax);
        furnace->addConstProperty("craft_speed",&FurnaceBlock::craftSpeed);
        furnace->addConstProperty("mesh",&FurnaceBlock::mesh);
        furnace->addConstProperty("texture",&FurnaceBlock::texture);
        furnace->setParent(block);

        TypeInfo* connected = registry.addTypeInfo<ConnectedBlock>("connected");
        connected->constructorFunction = [&]() {return std::make_unique<ConnectedBlock>();};
        connected->addConstProperty("texture",&ConnectedBlock::texture);
        connected->setParent(block);
    }

}