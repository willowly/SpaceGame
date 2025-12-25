#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"
#include <block/cockpit-block.hpp>
#include <block/thruster-block.hpp>
#include <block/furnace-block.hpp>

using std::string,std::variant;

namespace API {

    void loadBlockBaseType(ObjLoadType loadType,sol::table table,Block* block,Registry& registry) {
        switch (loadType) {
            case ObjLoadType::ARRAY:
                // type is in slot 1 (hopefully)
                getModel(table,2,&block->model,registry,false);
                getTexture(table,3,&block->texture,registry,false);
                break;
            case ObjLoadType::TABLE:
                getModel(table,"model",&block->model,registry,false);
                getTexture(table,"texture",&block->texture,registry,false);
                break;
            case ObjLoadType::INVALID:
                //registry.materials.erase(name);
                Debug::warn("trying to load with invalid object");
                break;
        }
    }

    void loadBlockThruster(ObjLoadType loadType,sol::table table,ThrusterBlock* block,Registry& registry) {
        loadBlockBaseType(loadType,table,block,registry);
        switch (loadType) {
            case ObjLoadType::ARRAY:
                get<float>(table,4,&block->force,false);
                break;
            case ObjLoadType::TABLE:
                get<float>(table,"force",&block->force,false);
                break;
            case ObjLoadType::INVALID:
                //registry.materials.erase(name);
                Debug::warn("trying to load with invalid object");
                break;
        }
    }
    
    void loadBlockCockpit(ObjLoadType loadType,sol::table table,CockpitBlock* block,Registry& registry) {
        loadBlockBaseType(loadType,table,block,registry);
    }
    void loadBlockFurnace(ObjLoadType loadType,sol::table table,FurnaceBlock* block,Registry& registry) {
        loadBlockBaseType(loadType,table,block,registry);
    }

    void addBlockWithTypeAndLoad(string type,string name,ObjLoadType loadType,sol::table table,Registry& registry) {
        if(type == "cockpit") {
            CockpitBlock* block = registry.addBlock<CockpitBlock>(name);
            loadBlockCockpit(loadType,table,block,registry);
            Debug::info("Loaded Cockpit Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "thruster") {
            ThrusterBlock* block = registry.addBlock<ThrusterBlock>(name);
            loadBlockThruster(loadType,table,block,registry);
            Debug::info("Loaded Thruster Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        if(type == "furnace") {
            FurnaceBlock* block = registry.addBlock<FurnaceBlock>(name);
            loadBlockFurnace(loadType,table,block,registry);
            Debug::info("Loaded Furnace Block \"" + name + "\"",InfoPriority::MEDIUM);
            return;
        }
        Block* block = registry.addBlock<Block>(name);
        loadBlockBaseType(loadType,table,block,registry);
        Debug::info("Loaded Block \"" + name + "\"",InfoPriority::MEDIUM);
        
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
            ObjLoadType loadType = getObjectLoadType(lua,obj);
            switch (getObjectLoadType(lua,obj)) {
                case ObjLoadType::ARRAY:
                    get<string>(table,1,&type,true);
                    break;
                case ObjLoadType::TABLE:
                    get<string>(table,"type",&type,true);
                    break;
                case ObjLoadType::INVALID:
                    type = "invalid";
                    Debug::warn("object is invalid");
                    break;
            }

            if(type != "invalid") {
                addBlockWithTypeAndLoad(type,name,loadType,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}