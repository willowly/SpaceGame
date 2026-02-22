#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"
#include <block/cockpit-block.hpp>
#include <block/thruster-block.hpp>
#include <block/furnace-block.hpp>
#include "block/connected-block.hpp"

using std::string,std::variant;

namespace API {


    // void loadBlockBaseType(ObjLoadType loadType,sol::table table,Block* block,Registry& registry) {
    //     switch (loadType) {
    //         case ObjLoadType::ARRAY:
    //             // type is in slot 1 (hopefully)
    //             getBlockModel(table,2,block,registry);
    //             getTexture(table,3,&block->texture,registry,false);
    //             break;
    //         case ObjLoadType::TABLE:
    //             getBlockModel(table,"model",block,registry);
    //             getTexture(table,"texture",&block->texture,registry,false);
    //             break;
    //         case ObjLoadType::INVALID:
    //             //registry.materials.erase(name);
    //             Debug::warn("trying to load with invalid object");
    //             break;
    //     }
    // }

    void loadBlockThruster(sol::table table,ThrusterBlock* block,Registry& registry) {
        getMesh(table,"mesh",block->mesh,registry,true);
        getTexture(table,"texture",block->texture,registry,true);
        get<float>(table,"force",block->force,false);
        get<float>(table,"side_force",block->sideForce,false);
    }

    void loadBlockConnected(sol::table table,ConnectedBlock* block,Registry& registry) {
        getTexture(table,"texture",block->texture,registry,true);
        get<bool>(table,"solid",block->solid,false);
    }
    
    void loadBlockCockpit(sol::table table,CockpitBlock* block,Registry& registry) {
        getMesh(table,"mesh",block->mesh,registry,true);
        getTexture(table,"texture",block->texture,registry,true);
    }
    void loadBlockFurnace(sol::table table,FurnaceBlock* block,Registry& registry) {
        getMesh(table,"mesh",block->mesh,registry,true);
        getTexture(table,"texture",block->texture,registry,true);
        get<float>(table,"fuel_max",block->fuelMax,false);
        get<float>(table,"speed",block->craftSpeed,false);
        // should be other stuff like speed etc
    }

    void addBlockWithTypeAndLoad(string type,string name,sol::table table,Registry& registry) {
        if(type == "cockpit") {
            CockpitBlock* block = registry.addBlock<CockpitBlock>(name);
            loadBlockCockpit(table,block,registry);
            Debug::info("Loaded Cockpit Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "thruster") {
            ThrusterBlock* block = registry.addBlock<ThrusterBlock>(name);
            loadBlockThruster(table,block,registry);
            Debug::info("Loaded Thruster Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "connected") {
            ConnectedBlock* block = registry.addBlock<ConnectedBlock>(name);
            loadBlockConnected(table,block,registry);
            Debug::info("Loaded Connected Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "furnace") {
            FurnaceBlock* block = registry.addBlock<FurnaceBlock>(name);
            loadBlockFurnace(table,block,registry);
            Debug::info("Loaded Furnace Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        Debug::warn("No fallback type for blocks");
        // Block* block = registry.addBlock<Block>(name);
        // loadBlockBaseType(loadType,table,block,registry);
        // Debug::info("Loaded Block \"" + name + "\"",InfoPriority::MEDIUM);
        
    }
    


    struct BlockRegistry {
        BlockRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Block* index(string name) {
            return registry.getBlock(name);
        }

        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("blocks");
            Debug::addTrace(name);
            sol::table table = obj; //this will be null if it doesnt work, so be careful

            string type = "";
            ObjLoadType loadType = getObjectLoadType(obj);
            switch (getObjectLoadType(obj)) {
                case ObjLoadType::ARRAY:
                    type = "invalid";
                    Debug::warn("can't load blocks in array mode");
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
                addBlockWithTypeAndLoad(type,name,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}