#pragma once

#include <sol/sol.hpp>

#include "engine/registry.hpp"
#include "actor/components/particle-effect.hpp"
#include "engine/debug.hpp"
#include <graphics/color.hpp>
#include <api/api-registry-general.hpp>

using std::string,std::variant;

namespace API {


    void getParticleFloatRange(sol::table table,std::variant<string,int> key,ParticleEffect::FloatRange& range,bool required = false) {
        Debug::addTrace(keyAsString(key)); 
        sol::object obj = table[key];
        if(obj == sol::lua_nil) {
            if(required) {
                Debug::warn("no value found for " + keyAsString(key));
            }
            Debug::subtractTrace();
            return;
        }
        if(obj.is<float>()) {
            range = obj.as<float>();
        }
        sol::table array = obj;
        if(array != sol::lua_nil && array[1] != sol::lua_nil && array[2] != sol::lua_nil) {
            sol::object start = array[1];
            sol::object end = array[2];
            if(start.is<float>()) {
                range.start = start.as<float>();
            } else {
                Debug::warn("Couldn't load float-range, invalid start");
            }
            if(end.is<float>()) {
                range.start = end.as<float>();
            } else {
                Debug::warn("Couldn't load float-range, invalid end");
            }
        } else {
            Debug::warn("Couldn't load float-range, invalid object");
        }
        Debug::subtractTrace();
        // we dont have an way to represent these in lua yet (and may never will) so we dont need to get
    }

    void loadParticleEffect(sol::table table,ParticleEffect& effect,Registry& registry) {
        getMesh(table,"mesh",effect.mesh,registry);
        getMaterial(table,"material",effect.material,registry);
        get<float>(table,"spawnRate",effect.spawnRate);
        get<int>(table,"initial_spawn_count",effect.initialSpawnCount);
        getParticleFloatRange(table,"initial_velocity",effect.initialVelocity);
        getParticleFloatRange(table,"life_time",effect.lifeTime);
        getParticleFloatRange(table,"particle_size",effect.particleSize);
        getParticleFloatRange(table,"initial_angular_velocity",effect.initialAngularVelocity);
        get<float>(table,"radius",effect.emitterShape.radius); //just for now lol, I dont want to deal with the shape stuff until I have reason to
    }

    struct ParticleEffectRegistry {
        ParticleEffectRegistry(Registry& registry) : registry(registry) {}
        Registry& registry;

        ParticleEffect* index(string name) {
            
            return registry.getParticleEffect(name);
        }
        
        void newindex(sol::this_state lua,string name,sol::object obj) {
            Debug::addTrace("particle_effects");
            Debug::addTrace(name);
            
            
            sol::table table = obj; //this will be null if it doesnt work, so be careful
            ParticleEffect effect;
            switch (getObjectLoadType(obj)) {
                
                case ObjLoadType::ARRAY:
                    Debug::warn("loading particle effects from arrays is not supported");
                    break;
                case ObjLoadType::TABLE:
                    loadParticleEffect(table,effect,registry);
                    registry.addParticleEffect(name,effect);
                    break;
                case ObjLoadType::INVALID:
                    Debug::warn("trying to load with invalid object");
                    break;
                    
            }
            Debug::info("Loaded Particle Effect \"" + name + "\"",InfoPriority::MEDIUM);
            Debug::subtractTrace();
            Debug::subtractTrace();
        }
    };

}