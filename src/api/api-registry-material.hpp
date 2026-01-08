#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include <api/api-registry-general.hpp>

using std::string,std::variant;

namespace API {


    struct MaterialRegistry {
        MaterialRegistry(Registry& registry,Vulkan* vulkan) : registry(registry), vulkan(vulkan) {}
        Registry& registry;
        Vulkan* vulkan;

        Material index(string name) {
            
            return registry.getMaterial(name);
        }
        
        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("materials");
            Debug::addTrace(name);
            
            
            sol::table table = obj; //this will be null if it doesnt work, so be careful
            LitMaterialData materialData;
            VkPipeline pipeline = registry.litShader;
            switch (getObjectLoadType(lua,obj)) {
                
                case ObjLoadType::ARRAY:
                    getTexture(table,2,&materialData.texture,registry,true);
                    getColorAsVec4(table,3,&materialData.color,false);
                    registry.addMaterial(name,vulkan->createMaterial(pipeline,materialData));
                    break;
                case ObjLoadType::TABLE:
                    getTexture(table,"texture",&materialData.texture,registry,true);
                    getColorAsVec4(table,"color",&materialData.color,false);
                    registry.addMaterial(name,vulkan->createMaterial(pipeline,materialData));
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

}