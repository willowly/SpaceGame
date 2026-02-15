#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>

using std::string,std::variant,glm::vec3;

namespace API {

    enum ObjLoadType {
        INVALID,
        ARRAY,
        TABLE
    };


    ObjLoadType getObjectLoadType(sol::object solObject) {

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
    void get(sol::table table,variant<string,int> key,T& ref,bool required = false) {
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
            ref = obj.as<T>();
        } else {
            Debug::warn("wrong type");
        }
        Debug::subtractTrace();
    }

    void getColorAsVec3(sol::table table,variant<string,int> key,vec3& ref,bool required = false) {
        Color color(ref);
        get<Color>(table,key,color,false);
        ref = color.asVec3();
    }
    void getColorAsVec4(sol::table table,variant<string,int> key,vec4& ref,bool required = false) {
        Color color(ref);
        get<Color>(table,key,color,false);
        ref = color.asVec4();
    }

    void getTexture(sol::table table,std::variant<string,int> key,TextureID& ref,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            ref = registry.getTexture(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<TextureID>(table,key,ref,required);
    }

    void getSprite(sol::table table,std::variant<string,int> key,Sprite& ref,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            ref = registry.getSprite(str);
            Debug::subtractTrace();
            return;
            
        }
        //TODO add extra here
        Debug::subtractTrace();
        get<Sprite>(table,key,ref,required);
    }

    void getMesh(sol::table table,std::variant<string,int> key,Mesh<Vertex>*& ref,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key));
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            ref = registry.getModel(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Mesh<Vertex>*>(table,key,ref,required);
    }

    //i actually think these should go in here, otherwise theres a dependancy nightmare

    Material createLitMaterial(sol::object obj,Registry& registry,Vulkan* vulkan) {
        LitMaterialData materialData;
        VkPipeline pipeline = registry.litShader;
        sol::table table = obj;
        switch (getObjectLoadType(obj)) {
            case ObjLoadType::ARRAY:
                getTexture(table,2,materialData.texture,registry,true);
                getColorAsVec4(table,3,materialData.color,false);
                break;
            case ObjLoadType::TABLE:
                getTexture(table,"texture",materialData.texture,registry,true);
                getColorAsVec4(table,"color",materialData.color,false);
                break;
            case ObjLoadType::INVALID:
                Debug::warn("object is invalid");
                break;
        }
        return vulkan->createMaterial(pipeline,materialData);
    }

    void getMaterial(sol::table table,std::variant<string,int> key,Material& ref,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key)); 
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            ref = registry.getMaterial(str);
            Debug::subtractTrace();
            return;
            
        }
        
        Debug::subtractTrace();
        get<Material>(table,key,ref,required);
    }

    void getBlock(sol::table table,std::variant<string,int> key,Block*& ref,Registry& registry,bool required = false) {
        Debug::addTrace(keyAsString(key)); 
        sol::object obj = table[key];
        if(obj.is<string>()) {
            string str = obj.as<string>();
            ref = registry.getBlock(str);
            Debug::subtractTrace();
            return;
            
        }
        Debug::subtractTrace();
        get<Block*>(table,key,ref,required);
    }

    // honestly fuck this for now
    // void getVec3(sol::table table,variant<string,int> key,vec3* ref,bool required = false) {
    //     Debug::addTrace(keyAsString(key)); 
    //     sol::table obj = table[key];
    //     if(obj != sol::lua_nil) {
    //         sol::object x = obj[1];
    //         sol::object y = obj[1];
    //         sol::object z = obj[1];
    //         if(x.is<)
    //         sol::table table = solObject;
    //         string str = obj.as<string>();
    //         *ref = registry.getBlock(str);
    //         Debug::subtractTrace();
    //         return;
            
    //     }
    //     Debug::subtractTrace();
    //     get<Block*>(table,key,ref,required);
    // }



    string getType(sol::object obj) {
        string type;
        ObjLoadType loadType = getObjectLoadType(obj);
        sol::table table = obj;
        switch (loadType) {
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

    struct SpriteRegistry {
        SpriteRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        Sprite index(string name) {
            return registry.getSprite(name);
        }
        
        void newindex(string name,sol::object obj) {
            throw std::runtime_error("Cannot modify sprite registry (just yet)");
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