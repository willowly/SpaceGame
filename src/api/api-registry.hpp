#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"

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
            if(registry.textures.contains(str)) {
                *pointer = &registry.textures.at(str);
            } else {
                Debug::warn("no texture called " + str);
            }
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Texture*>(table,key,pointer,required);
    }

    //these are all seperate to make it easier to access
    struct TextureRegistry {
        TextureRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Texture* index(string name) {
            if(registry.textures.contains(name)) {
                return &registry.textures.at(name);
            }
            Debug::warn("no texture called " + name);
            return nullptr;
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
            
            if(registry.materials.contains(name)) {
                return &registry.materials.at(name);
            }
            return nullptr;
        }
        
        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("materials");
            Debug::addTrace(name);
            registry.materials[name] = Material();
            Material& material = registry.materials[name];
            sol::table table = obj; //this will be null if it doesnt work, so be careful
            switch (getObjectLoadType(lua,obj)) {
                case ObjLoadType::ARRAY:
                    getShader(table,1,&material.shader,registry,true);
                    getTexture(table,2,&material.texture,registry,true);
                    break;
                case ObjLoadType::TABLE:
                    getShader(table,"shader",&material.shader,registry,true);
                    getTexture(table,"texture",&material.texture,registry,true);
                    break;
                case ObjLoadType::INVALID:
                    registry.materials.erase(name);
                    Debug::warn("trying to load with invalid object");
                    break;
            }
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

    struct PrototypeRegistry {
        PrototypeRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;
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

    }

}