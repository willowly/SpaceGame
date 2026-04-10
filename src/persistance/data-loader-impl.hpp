#pragma once
#include "actor/actors-all.hpp"
#include "actor/data-actor.hpp"
#include "helper/string-helper.hpp"
#include "engine/registry.hpp"
#include "graphics/vulkan.hpp"
#include "data-loader.hpp"

class DataLoaderImpl : public DataLoader {

    Registry& registry;
    Material constructionMaterial;

    public:
        DataLoaderImpl(Registry& registry,Material constructionMaterial) : registry(registry), constructionMaterial(constructionMaterial) {}

        template <typename T, typename data_T, bool includeLoader = false>
        std::unique_ptr<Actor> actorLoadSpecPrototype(data_ActorEntry entry) {
            try {
                std::cout << "prototype: " << (string)entry.name << std::endl;
                data_T* data = cista::deserialize<data_T>(entry.data);

                if constexpr (includeLoader) {
                    return T::makeInstanceFromSave(*data,registry.getActor<T>((string)entry.name),*this);
                } else {
                    return T::makeInstanceFromSave(*data,registry.getActor<T>((string)entry.name));
                }
            } catch(std::runtime_error e) {
                std::cout << "Failed to deserialize actor data from buffer: "<< e.what() << std::endl;
            }

            return nullptr;
        }

        std::unique_ptr<Actor> loadActor(data_ActorEntry entry) {
            
            
            switch (entry.type)
            {
                case data_ActorType::PLAYER:
                    
                    return actorLoadSpecPrototype<Character,data_Character,true>(entry);
                case data_ActorType::PHYSICS:
                    return actorLoadSpecPrototype<RigidbodyActor,data_RigidbodyActor>(entry);
                 case data_ActorType::CONSTRUCTION:
                    try {
                        data_Construction* data = cista::deserialize<data_Construction>(entry.data);
                        auto construction = Construction::makeInstanceFromSave(*data,constructionMaterial,*this);
                        return construction;
                    } catch(std::runtime_error e) {
                        std::cout << "Failed to deserialize actor data from buffer: "<< e.what() << std::endl;
                    }
                    break;
                
                default:
                    std::cout << "INVALID ACTOR";
                    break;
            }
            return nullptr;
        }

        Actor* getActorPrototype(string name) {
            return registry.getActor(name);
        }

        Item* getItemPrototype(string name) {
            return registry.getItem(name);
        }

        Block* getBlockPrototype(string name) {
            return registry.getBlock(name);
        }

};