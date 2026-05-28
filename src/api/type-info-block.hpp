#pragma once
#include "type-info.hpp"
#include "block/blocks-all.hpp"


namespace TypeInfoLoader {

    inline void loadBlock(Registry& registry) {
        
        TypeInfo* block = registry.addTypeInfo("block");
        block->addConstProperty("default_drop",&Block::defaultDrop); 
        block->addConstProperty("solid",&Block::solid);

        TypeInfo* thruster = registry.addTypeInfo("thruster");
        thruster->addConstProperty("mesh",&ThrusterBlock::mesh);
        thruster->addConstProperty("texture",&ThrusterBlock::texture);
        thruster->parent = block;

        TypeInfo* cockpit = registry.addTypeInfo("cockpit");
        cockpit->addConstProperty("mesh",&CockpitBlock::mesh);
        cockpit->addConstProperty("texture",&CockpitBlock::texture);
        cockpit->parent = block;

        TypeInfo* furnace = registry.addTypeInfo("furnace");
        furnace->addConstProperty("fuel_max",&FurnaceBlock::fuelMax);
        furnace->addConstProperty("craft_speed",&FurnaceBlock::craftSpeed);
        furnace->addConstProperty("mesh",&FurnaceBlock::mesh);
        furnace->addConstProperty("texture",&FurnaceBlock::texture);
        furnace->parent = block;

        TypeInfo* connected = registry.addTypeInfo("connected");
        connected->addConstProperty("texture",&ConnectedBlock::texture);
        connected->parent = block;
    }

}