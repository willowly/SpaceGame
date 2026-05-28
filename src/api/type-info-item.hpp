#pragma once
#include "type-info.hpp"
#include "item/items-all.hpp"


namespace TypeInfoLoader {

    inline void loadItem(Registry& registry) {

        TypeInfo* recipe = registry.addTypeInfo("recipe");
        recipe->addConstProperty("category",&Recipe::category);
        recipe->addConstProperty("time",&Recipe::time);
        recipe->addConstProperty("ingredients",&Recipe::ingredients);
        recipe->addConstProperty("result",&Recipe::result);
        
        TypeInfo* item = registry.addTypeInfo("item");
        item->addConstProperty("display_name",&Item::displayName); 
        item->addConstProperty("default_sprite",&Item::defaultSprite);
        item->addConstProperty("default_model",&Item::defaultModel);
        item->addConstProperty("default_material",&Item::defaultMaterial);
        item->addConstProperty("default_model_scale",&Item::defaultModelScale);

        TypeInfo* resource = registry.addTypeInfo("resource_item");
        resource->parent = item;

        TypeInfo* tool = registry.addTypeInfo("tool");
        tool->parent = item;
        tool->addConstProperty("held_model",&Tool::heldModel);
        tool->addConstProperty("held_model_material",&Tool::heldModelMaterial);
        tool->addConstProperty("model_offset",&Tool::modelOffset);
        tool->addConstProperty("model_rotation",&Tool::modelRotation);
        tool->addConstProperty("model_scale",&Tool::modelScale); 
        tool->addConstProperty("smooth_time",&Tool::smoothTime);
        
        TypeInfo* place = registry.addTypeInfo("place_block_tool");
        place->parent = tool;
        place->addConstProperty("place_animation_rotation",&PlaceBlockTool::placeAnimationRotation);
        place->addConstProperty("place_animation_time",&PlaceBlockTool::placeAnimationTime);
        place->addConstProperty("block",&PlaceBlockTool::block);

        TypeInfo* pickaxe = registry.addTypeInfo("pickaxe_tool");
        pickaxe->parent = tool;
        pickaxe->addConstProperty("durability",&PickaxeTool::durability);
        pickaxe->addConstProperty("mine_radius",&PickaxeTool::mineRadius);
        pickaxe->addConstProperty("mine_amount",&PickaxeTool::mineAmount);
        pickaxe->addConstProperty("reach",&PickaxeTool::reach);

        pickaxe->addConstProperty("anticipation_time",&PickaxeTool::anticipationTime);
        pickaxe->addConstProperty("cooldown_time",&PickaxeTool::cooldownTime);

        pickaxe->addConstProperty("anticipation_rotation",&PickaxeTool::anticipationRotation);
        pickaxe->addConstProperty("cooldown_rotation",&PickaxeTool::cooldownRotation);
    }

}