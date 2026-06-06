#pragma once
#include "type-info.hpp"
#include "block/blocks-all.hpp"


namespace TypeInfoLoader {

    inline void loadBlock(Registry& registry) {
        
        TypeInfo* block = registry.addTypeInfo<Block>("block");
        block->addConstProperty("default_drop",&Block::defaultDrop); 
        block->addConstProperty("solid",&Block::solid);

        TypeInfo* thruster = registry.addTypeInfo<ThrusterBlock>("thruster");
        thruster->addConstProperty("mesh",&ThrusterBlock::mesh);
        thruster->addConstProperty("texture",&ThrusterBlock::texture);
        thruster->parent = block;

        TypeInfo* cockpit = registry.addTypeInfo<CockpitBlock>("cockpit");
        cockpit->addConstProperty("mesh",&CockpitBlock::mesh);
        cockpit->addConstProperty("texture",&CockpitBlock::texture);
        cockpit->parent = block;

        TypeInfo* furnace = registry.addTypeInfo<FurnaceBlock>("furnace");
        furnace->addConstProperty("fuel_max",&FurnaceBlock::fuelMax);
        furnace->addConstProperty("craft_speed",&FurnaceBlock::craftSpeed);
        furnace->addConstProperty("mesh",&FurnaceBlock::mesh);
        furnace->addConstProperty("texture",&FurnaceBlock::texture);
        furnace->parent = block;

        TypeInfo* connected = registry.addTypeInfo<ConnectedBlock>("connected");
        connected->addConstProperty("texture",&ConnectedBlock::texture);
        connected->parent = block;
    }

}