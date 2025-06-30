#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>

using std::string,std::variant;

namespace API {

    enum ObjLoadType {
        INVALID,
        ARRAY,
        TABLE
    };


    ObjLoadType getObjectLoadType(sol::this_state& thisState,sol::object solObject) {

        sol::table table = solObject;
        if(table == sol::lua_nil) {
            return ObjLoadType::INVALID;
        }

        if(table[1] != sol::lua_nil) {
            return ObjLoadType::ARRAY;
        }

        return ObjLoadType::TABLE;
    };

    string keyAsString(std::variant<string,int> key) {
        if(key.index() == 0) {
            return get<string>(key);
        } else {
            return std::to_string(get<int>(key));
        }
    }

    template<typename T> 
    void get(sol::table table,variant<string,int> key,T* pointer,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj == sol::lua_nil) {
            if(required) {
                Debug::warn("no value found for " + keyAsString(key));
            }
            Debug::subtractTrace();
            return;
        }
        if(obj.is<T>()) {
            *pointer = obj.as<T>();
        } else {
            Debug::warn("wrong type");
        }
        Debug::subtractTrace();
    }

    void getShader(sol::table table,std::variant<string,int> key,Shader** pointer,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            if(str == "lit") {
                *pointer = &registry.litShader;
            } 
            else {
                Debug::warn("no shader called " + str);
                *pointer = &registry.litShader;
            }
            Debug::subtractTrace();
            return;
        }
        Debug::subtractTrace();
        get<Shader*>(table,key,pointer,required);
    }

    void getTexture(sol::table table,std::variant<string,int> key,Texture** pointer,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            *pointer = registry.getTexture(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Texture*>(table,key,pointer,required);
    }
    void getMaterial(sol::table table,std::variant<string,int> key,Material** pointer,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            *pointer = registry.getMaterial(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Material*>(table,key,pointer,required);
    }

    void getModel(sol::table table,std::variant<string,int> key,Model** pointer,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            *pointer = registry.getModel(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Model*>(table,key,pointer,required);
    }

    

    void loadActorDummyType(ObjLoadType loadType,sol::table table,Actor* actor,Registry& registry) {
        switch (loadType) {
            case ObjLoadType::ARRAY:
                // type is in slot 1 (hopefully)
                getModel(table,2,&actor->model,registry,false);
                getMaterial(table,3,&actor->material,registry,false);
                break;
            case ObjLoadType::TABLE:
                getModel(table,"model",&actor->model,registry,false);
                getMaterial(table,"material",&actor->material,registry,false);
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

    //these are all seperate to make it easier to access
    struct TextureRegistry {
        TextureRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Texture* index(string name) {
            return registry.getTexture(name);
        }
        
        void newindex(string name,sol::object obj) {
            throw std::runtime_error("Cannot modify texture registry");
        }
    };

    struct ShaderRegistry {
        ShaderRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Shader* litShader() {
            return &registry.litShader;
        }
    };

    struct ModelRegistry {
        ModelRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;
        
    };

    struct MaterialRegistry {
        MaterialRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Material* index(string name) {
            
            return registry.getMaterial(name);
        }
        
        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("materials");
            Debug::addTrace(name);
            registry.addMaterial(name);
            Material* material = registry.getMaterial(name);
            sol::table table = obj; //this will be null if it doesnt work, so be careful
            switch (getObjectLoadType(lua,obj)) {
                case ObjLoadType::ARRAY:
                    getShader(table,1,&material->shader,registry,true);
                    getTexture(table,2,&material->texture,registry,true);
                    get<Color>(table,3,&material->color,false);
                    break;
                case ObjLoadType::TABLE:
                    getShader(table,"shader",&material->shader,registry,true);
                    getTexture(table,"texture",&material->texture,registry,true);
                    get<Color>(table,"color",&material->color,false);
                    break;
                case ObjLoadType::INVALID:
                    //registry.materials.erase(name);
                    Debug::warn("trying to load with invalid object");
                    break;
                    
            }
            Debug::info("Loaded Material \"" + name + "\"",InfoPriority::MEDIUM);
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

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
                if(type == "character") {

                }
                if(type == "rigidbody") {

                }
                addActorWithTypeAndLoad(type,name,loadType,table,registry);
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

    void loadAPIRegistry(sol::state& lua) {

        sol::usertype<TextureRegistry> textureRegistry = lua.new_usertype<TextureRegistry>("textureRegistry",sol::no_constructor);

        textureRegistry["__index"] = &TextureRegistry::index;
        textureRegistry["__newindex"] = &TextureRegistry::newindex;

        sol::usertype<ShaderRegistry> shaderRegistry = lua.new_usertype<ShaderRegistry>("shaderRegistry",sol::no_constructor);

        shaderRegistry["lit"] = sol::property(&ShaderRegistry::litShader);

        sol::usertype<MaterialRegistry> materialRegistry = lua.new_usertype<MaterialRegistry>("materialRegistry",sol::no_constructor);

        materialRegistry["__index"] = &MaterialRegistry::index;
        materialRegistry["__newindex"] = &MaterialRegistry::newindex;

        sol::usertype<ActorRegistry> actorRegistry = lua.new_usertype<ActorRegistry>("actorRegistry",sol::no_constructor);

        actorRegistry["__index"] = &ActorRegistry::index;
        actorRegistry["__newindex"] = &ActorRegistry::newindex;

    }

}