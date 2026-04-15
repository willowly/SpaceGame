#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"
#include <item/items-all.hpp>

using std::string,std::variant;

namespace API {

    inline void loadItemBaseType(ObjLoadType loadType,sol::table table,Item* item,Registry& registry) {
        get<string>(table,"display_name",item->displayName,true);
        getSprite(table,"icon",item->defaultSprite,registry,true);
        item->defaultModel = registry.getModel("item_default");
        item->defaultMaterial = registry.getMaterial("item_default");
        getMesh(table,"model",item->defaultModel,registry,false);
        getMaterial(table,"material",item->defaultMaterial,registry,false);
    }

    inline void loadItemToolType(ObjLoadType loadType,sol::table table,Tool* item,Registry& registry) {
        loadItemBaseType(loadType,table,item,registry);
        get<float>(table,"look_lerp",item->lookLerp,false);
        getMesh(table,"model",item->heldModel,registry,false);
        getMaterial(table,"material",item->heldModelMaterial,registry,false);
        getMesh(table,"held_model",item->heldModel,registry,false);
        getMaterial(table,"held_material",item->heldModelMaterial,registry,false);
        get<vec3>(table,"offset",item->modelOffset,false);
        get<quat>(table,"rotation",item->modelRotation,false);
        get<float>(table,"scale",item->modelScale,false);
    }

    inline void loadItemPlaceTool(ObjLoadType loadType,sol::table table,PlaceBlockTool* item,Registry& registry) {
        loadItemToolType(loadType,table,item,registry);
        getBlock(table,"block",item->block,registry,false);
    }

    inline void loadItemPickaxe(ObjLoadType loadType,sol::table table,PickaxeTool* item,Registry& registry) {
        loadItemToolType(loadType,table,item,registry);
        get<float>(table,"mine_amount",item->mineAmount,false);
        get<float>(table,"mine_radius",item->mineRadius,false);
        get<int>(table,"durability",item->durability,false);
        // this will all become part of the animation system
        get<quat> (table,"anticipation_rotation",item->anticipationRotation,false);
        get<float>(table,"anticipation_time",    item->anticipationTime,false); 
        get<quat> (table,"cooldown_rotation",item->cooldownRotation,false);
        get<float>(table,"cooldown_time",    item->cooldownTime,false);
    }

    inline void loadItemDrill(ObjLoadType loadType,sol::table table,DrillTool* item,Registry& registry) {
        loadItemToolType(loadType,table,item,registry);
        get<float>(table,"mine_amount",item->mineAmount,false);
        get<float>(table,"mine_radius",item->mineRadius,false);
        get<int>(table,"durability",item->durability,false);
    }

    inline void addItemWithTypeAndLoad(string type,string name,ObjLoadType loadType,sol::table table,Registry& registry) {
        if(loadType == ObjLoadType::INVALID) {
            Debug::warn("trying to load with invalid object");
            return;
        }
        if(loadType == ObjLoadType::ARRAY) {
            Debug::warn("loading items in array mode is not supported");
        }

        if(type == "placetool") {
            PlaceBlockTool* item = registry.addItem<PlaceBlockTool>(name);
            loadItemPlaceTool(loadType,table,item,registry);
            Debug::info("Loaded Place Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "pickaxe") {
            PickaxeTool* item = registry.addItem<PickaxeTool>(name);
            loadItemPickaxe(loadType,table,item,registry);
            Debug::info("Loaded Pickaxe Tool \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "drill") {
            DrillTool* item = registry.addItem<DrillTool>(name);
            loadItemDrill(loadType,table,item,registry);
            Debug::info("Loaded Drill Tool \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        Item* block = registry.addItem<ResourceItem>(name);
        loadItemBaseType(loadType,table,block,registry);
        Debug::info("Loaded Item \"" + name + "\"",InfoPriority::MEDIUM);
        
    }


    struct ItemRegistry {
        ItemRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Item* index(string name) {
            return registry.getItem(name);
        }

        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("items");
            Debug::addTrace(name);
            sol::table table = obj; //this will be null if it doesnt work, so be careful

            string type = "";
            ObjLoadType loadType = getObjectLoadType(obj);
            switch (getObjectLoadType(obj)) {
                case ObjLoadType::ARRAY:
                    get<string>(table,1,type,true);
                    break;
                case ObjLoadType::TABLE:
                    get<string>(table,"type",type,true);
                    break;
                case ObjLoadType::INVALID:
                    type = "invalid";
                    Debug::warn("object is invalid");
                    break;
            }

            if(type != "invalid") {
                addItemWithTypeAndLoad(type,name,loadType,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}