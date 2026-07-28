#pragma once
#include "type-info.hpp"
#include "item/items-all.hpp"


namespace TypeInfoLoader {

    inline void loadItem(Registry& registry) {

        TypeInfo* recipe = registry.addTypeInfo<Recipe>("recipe");
        recipe->constructorFunction = [&]() {return std::make_unique<Recipe>();};
        recipe->addConstProperty("category",&Recipe::category);
        recipe->addConstProperty("time",&Recipe::time);
        recipe->addConstProperty("ingredients",&Recipe::ingredients);
        recipe->addConstProperty("result",&Recipe::result);

        TypeInfo* itemStack = registry.addTypeInfo<ItemStack>("item_stack");
        itemStack->addConstProperty("item",&ItemStack::item);
        itemStack->addConstProperty("amount",&ItemStack::amount);
        
        TypeInfo* item = registry.addTypeInfo<Item>("item");
        item->constructorFunction = [&]() {return std::make_unique<Item>();};
        item->addConstProperty("display_name",&Item::displayName); 
        item->addConstProperty("default_sprite",&Item::defaultSprite);
        item->addConstProperty("default_model",&Item::defaultModel);
        item->addConstProperty("default_material",&Item::defaultMaterial);
        item->addConstProperty("default_model_scale",&Item::defaultModelScale);

        TypeInfo* resource = registry.addTypeInfo<ResourceItem>("resource_item");
        resource->constructorFunction = [&]() {return std::make_unique<ResourceItem>();};
        resource->setParent(item);

        TypeInfo* tool = registry.addTypeInfo<Tool>("tool");
        tool->setParent(item);
        tool->addConstProperty("held_model",&Tool::heldModel);
        tool->addConstProperty("held_model_material",&Tool::heldModelMaterial);
        tool->addConstProperty("model_offset",&Tool::modelOffset);
        tool->addConstProperty("model_rotation",&Tool::modelRotation);
        tool->addConstProperty("model_scale",&Tool::modelScale); 
        tool->addConstProperty("smooth_time",&Tool::smoothTime);
        
        TypeInfo* place = registry.addTypeInfo<PlaceBlockTool>("place_block_tool");
        place->setParent(tool);
        place->constructorFunction = [&]() {return std::make_unique<PlaceBlockTool>();};
        place->addConstProperty("place_animation_rotation",&PlaceBlockTool::placeAnimationRotation);
        place->addConstProperty("place_animation_time",&PlaceBlockTool::placeAnimationTime);
        place->addConstProperty("block",&PlaceBlockTool::block);

        TypeInfo* hammer = registry.addTypeInfo<HammerTool>("hammer_tool");
        hammer->setParent(tool);
        hammer->constructorFunction = [&]() {return std::make_unique<HammerTool>();};
        
        TypeInfo* pickaxe = registry.addTypeInfo<PickaxeTool>("pickaxe_tool");
        pickaxe->setParent(tool);
        pickaxe->constructorFunction = [&]() {return std::make_unique<PickaxeTool>();};
        pickaxe->addConstProperty("durability",&PickaxeTool::durability);
        pickaxe->addConstProperty("mine_radius",&PickaxeTool::mineRadius);
        pickaxe->addConstProperty("mine_amount",&PickaxeTool::mineAmount);
        pickaxe->addConstProperty("reach",&PickaxeTool::reach);

        pickaxe->addConstProperty("anticipation_time",&PickaxeTool::anticipationTime);
        pickaxe->addConstProperty("cooldown_time",&PickaxeTool::cooldownTime);

        pickaxe->addConstProperty("anticipation_rotation",&PickaxeTool::anticipationRotation);
        pickaxe->addConstProperty("cooldown_rotation",&PickaxeTool::cooldownRotation);

        TypeInfo* clawTool = registry.addTypeInfo<ClawTool>("claw_tool");
        clawTool->setParent(tool);
        clawTool->constructorFunction = [&]() {return std::make_unique<ClawTool>();};
        clawTool->addConstProperty("range",&ClawTool::range);
        clawTool->addConstProperty("force",&ClawTool::force);
        clawTool->addConstProperty("velocity_threshold",&ClawTool::velocityThreshold);
        clawTool->addConstProperty("angular_damping",&ClawTool::angularDamping);
        clawTool->addConstProperty("scroll_speed",&ClawTool::scrollSpeed);

        TypeInfo* drillTool = registry.addTypeInfo<DrillTool>("drill_tool");
        drillTool->setParent(tool);
        drillTool->constructorFunction = [&]() {return std::make_unique<DrillTool>();};

        // work on building

        // TypeInfo* newTool = registry.addTypeInfo<NewTool>("new_tool");
        // newTool->setParent(tool);
        // newTool->constructorFunction = [&]() {return std::make_unique<NewTool>();};
    }

}