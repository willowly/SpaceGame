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
        thruster->addConstProperty("thrust_effect",&ThrusterBlock::thrustEffect);
        thruster->addConstProperty("small_thrust_effect",&ThrusterBlock::smallThrustEffect);
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
        furnace->addConstProperty("widget",&FurnaceBlock::widget);
        furnace->setParent(block);

        TypeInfo* connected = registry.addTypeInfo<ConnectedBlock>("connected");
        connected->constructorFunction = [&]() {return std::make_unique<ConnectedBlock>();};
        connected->addConstProperty("texture",&ConnectedBlock::texture);
        connected->addConstProperty("slope",&ConnectedBlock::slope);
        connected->setParent(block);

        TypeInfo* sloped = registry.addTypeInfo<SlopedBlock>("sloped");
        sloped->constructorFunction = [&]() {return std::make_unique<SlopedBlock>();};
        sloped->addConstProperty("texture",&SlopedBlock::texture);
        sloped->setParent(block);

        TypeInfo* drill = registry.addTypeInfo<DrillBlock>("drill");
        drill->constructorFunction = [&]() {return std::make_unique<DrillBlock>();};
        drill->addConstProperty("mesh",&DrillBlock::mesh);
        drill->addConstProperty("texture",&DrillBlock::texture);
        drill->addConstProperty("range",&DrillBlock::range);
        drill->addConstProperty("amount",&DrillBlock::amount);
        drill->addConstProperty("radius",&DrillBlock::radius);
        drill->setParent(block);

        TypeInfo* container = registry.addTypeInfo<ContainerBlock>("container");
        container->constructorFunction = [&]() { return std::make_unique<ContainerBlock>(); };
        container->addConstProperty("texture",&ContainerBlock::texture);
        container->addConstProperty("widget",&ContainerBlock::widget);
        container->addConstProperty("max_weight",&ContainerBlock::maxWeight);
        container->setParent(block);

    }

}