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

    void getColorAsVec3(sol::table table,variant<string,int> key,vec3* pointer,bool required = false) {
        Color color;
        get<Color>(table,"color",&color,false);
        *pointer = color.asVec3();
    }
    void getColorAsVec4(sol::table table,variant<string,int> key,vec4* pointer,bool required = false) {
        Color color;
        get<Color>(table,"color",&color,false);
        *pointer = color.asVec4();
    }

    void getTexture(sol::table table,std::variant<string,int> key,TextureID* pointer,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            *pointer = registry.getTexture(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<TextureID>(table,key,pointer,required);
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

    string getType(sol::this_state& thisState,sol::object obj) {
        string type;
        ObjLoadType loadType = getObjectLoadType(thisState,obj);
        sol::table table = obj;
        switch (loadType) {
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
        return type;
    }

    //these are all seperate to make it easier to access
    struct TextureRegistry {
        TextureRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        TextureID index(string name) {
            return registry.getTexture(name);
        }
        
        void newindex(string name,sol::object obj) {
            throw std::runtime_error("Cannot modify texture registry");
        }
    };

    // struct ShaderRegistry {
    //     ShaderRegistry(Registry& registry) : registry(registry) {}
    //     Registry& registry;

    //     Shader* litShader() {
    //         return &registry.litShader;
    //     }
    // };

    struct ModelRegistry {
        ModelRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;
        
    };


}