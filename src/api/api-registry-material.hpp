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

            Material material = createLitMaterial(obj,registry,vulkan);
            if(material.isValid()) {
                Debug::info("Loaded Material \"" + name + "\"",InfoPriority::MEDIUM);
                registry.addMaterial(name,material);
            }
            // we dont really need to have a warning here since createLitMaterial should emit one (thats more helpful)
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

}