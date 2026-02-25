#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"

using std::string,std::variant;

namespace API {

    void loadActorDummyType(ObjLoadType loadType,sol::table table,Actor* actor,Registry& registry) {
        switch (loadType) {
            case ObjLoadType::ARRAY:
                // type is in slot 1 (hopefully)
                getMesh(table,2,actor->model,registry,false);
                getMaterial(table,3,actor->material,registry,false);
                break;
            case ObjLoadType::TABLE:
                getMesh(table,"model",actor->model,registry,false);
                getMaterial(table,"material",actor->material,registry,false);
                break;
            case ObjLoadType::INVALID:
                //registry.materials.erase(name);
                Debug::warn("trying to load with invalid object");
                break;
        }
    }

    void addActorWithTypeAndLoad(string type,string name,ObjLoadType loadType,sol::table table,Registry& registry) {
        if(type == "character") {

        }
        if(type == "rigidbody") {

        }
        Actor* actor = registry.addActor<Actor>(name);
        loadActorDummyType(loadType,table,actor,registry);
        Debug::info("Loaded Actor \"" + name + "\"",InfoPriority::MEDIUM);
        
    }


    struct ActorRegistry {
        ActorRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Actor* index(string name) {
            return registry.getActor(name);
        }

        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("actors");
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
                addActorWithTypeAndLoad(type,name,loadType,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}