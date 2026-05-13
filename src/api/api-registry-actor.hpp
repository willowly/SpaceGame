#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include "api/api-registry.hpp"

using std::string,std::variant;

namespace API {

    inline void loadActorDummyType(sol::table table,Actor* actor,Registry& registry) {
        getMesh(table,"model",actor->model,registry,false);
        getMaterial(table,"material",actor->material,registry,false);
    }

    inline void loadActorCharacter(sol::table table,Character* actor,Registry& registry) {
        loadActorDummyType(table,actor,registry);
        get<float>(table,"move_speed",actor->moveSpeed);
        get<float>(table,"look_sensitivity",actor->lookSensitivity);
        get<float>(table,"height",actor->height);
        get<float>(table,"radius",actor->radius);
        get<float>(table,"ground_acceleration",actor->groundAcceleration);
        get<float>(table,"ground_decelleration",actor->groundDecelleration);
        get<float>(table,"air_acceleration",actor->airAcceleration);
        get<float>(table,"jump_force",actor->jumpForce);
        get<float>(table,"craft_speed",actor->craftSpeed);
        get<float>(table,"rotation_speed",actor->rotationSpeed);
        get<float>(table,"rotation_acceleration",actor->rotationAcceleration);
        get<float>(table,"item_dropDistance",actor->itemDropDistance);
        get<vec3>(table,"third_person_camera_cffset",actor->thirdPersonCameraOffset);
        get<vec3>(table,"third_person_camera_rot",actor->thirdPersonCameraRot);
        get<float>(table,"input_buffer_time",actor->inputBuffer);
        get<float>(table,"coyote_time",actor->coyoteTime);
        get<float>(table,"camera_clear_radius",actor->cameraClearRadius);
        get<bool>(table,"inventory_disabled",actor->inventoryDisabled);
    }
    
    inline void addActorWithTypeAndLoad(string type,string name,sol::table table,Registry& registry) {
        if(type == "character") {
            Character* actor = registry.addActor<Character>(name);
            loadActorCharacter(table,actor,registry);
        }
        else if(type == "rigidbody") {
            
        } else {
            Actor* actor = registry.addActor<Actor>(name);
            loadActorDummyType(table,actor,registry);
        }
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
                    type = "invalid";
                    Debug::warn("Cannot load actors in array mode");
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
                addActorWithTypeAndLoad(type,name,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };
}