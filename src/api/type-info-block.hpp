#pragma once
#include "type-info.hpp"
#include "block/blocks-all.hpp"


namespace TypeInfoLoader {

    inline void loadBlock(Registry& registry) {


        TypeInfo* fuelBurner = registry.addTypeInfo<FuelBurner>("fuel_burner");
        fuelBurner->addConstProperty("burn_speed",&FuelBurner::burnSpeed);
        fuelBurner->addConstProperty("smart_fuel",&FuelBurner::smartFuel);
        
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

        TypeInfo* solarPanel = registry.addTypeInfo<SolarPanelBlock>("solar_panel");
        solarPanel->constructorFunction = [&]() {return std::make_unique<SolarPanelBlock>();};
        solarPanel->addConstProperty("mesh",&SolarPanelBlock::mesh);
        solarPanel->addConstProperty("texture",&SolarPanelBlock::texture);
        solarPanel->addConstProperty("power_output",&SolarPanelBlock::powerOutput);
        solarPanel->addConstProperty("max_charge",&SolarPanelBlock::maxCharge);
        solarPanel->setParent(block);

        TypeInfo* battery = registry.addTypeInfo<BatteryBlock>("battery");
        battery->constructorFunction = [&]() {return std::make_unique<BatteryBlock>();};
        battery->addConstProperty("texture",&BatteryBlock::texture);
        battery->addConstProperty("max_charge",&BatteryBlock::maxCharge);
        battery->addConstProperty("widget",&BatteryBlock::widget);
        battery->setParent(block);

        TypeInfo* furnace = registry.addTypeInfo<CrafterBlock>("crafter");
        furnace->constructorFunction = [&]() {return std::make_unique<CrafterBlock>();};
        furnace->addConstProperty("craft_speed",&CrafterBlock::craftSpeed);
        furnace->addConstProperty("mesh",&CrafterBlock::mesh);
        furnace->addConstProperty("texture",&CrafterBlock::texture);
        furnace->addConstProperty("widget",&CrafterBlock::widget);
        furnace->addConstProperty("category",&CrafterBlock::category);
        furnace->addConstProperty("max_ingredients",&CrafterBlock::maxIngredients);
        furnace->addConstProperty("fuel_burner",&CrafterBlock::fuelBurner);
        furnace->addConstProperty("electric",&CrafterBlock::electric);
        furnace->addConstProperty("electric_use_speed",&CrafterBlock::electricUseSpeed);
        furnace->addConstProperty("allow_manual",&CrafterBlock::allowManual);
        furnace->addConstProperty("manual_progress",&CrafterBlock::manualProgress);
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
        drill->addConstProperty("widget",&DrillBlock::widget);
        drill->addConstProperty("max_weight",&DrillBlock::maxWeight);
        drill->addConstProperty("drill_head_model",&DrillBlock::drillHeadModel);
        drill->addConstProperty("drill_rotate_speed",&DrillBlock::drillRotateSpeed);
        drill->setParent(block);

        TypeInfo* container = registry.addTypeInfo<ContainerBlock>("container");
        container->constructorFunction = [&]() { return std::make_unique<ContainerBlock>(); };
        container->addConstProperty("texture",&ContainerBlock::texture);
        container->addConstProperty("widget",&ContainerBlock::widget);
        container->addConstProperty("max_weight",&ContainerBlock::maxWeight);
        container->setParent(block);

    }

}