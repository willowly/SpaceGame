#pragma once
#include "actor/actors-all.hpp"
#include "actor/data-actor.hpp"
#include "helper/string-helper.hpp"
#include "engine/registry.hpp"
#include "graphics/vulkan.hpp"
#include "data-loader.hpp"
#include "item/recipe.hpp"

class DataLoaderImpl : public DataLoader {

    Registry& registry;

    public:
        DataLoaderImpl(Registry& registry) : registry(registry) {}

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

        template <typename T, typename data_T>
        std::unique_ptr<Actor> actorLoadGeneric(data_ActorEntry entry) {
            try {
                data_ItemActor* data = cista::deserialize<data_ItemActor>(entry.data);
                return T::makeInstanceFromSave(*data,*this);
            } catch(std::runtime_error e) {
                std::cout << "Failed to deserialize actor data from buffer: "<< e.what() << std::endl;
            }
            return nullptr;
        }

        std::unique_ptr<Actor> loadActor(data_ActorEntry entry) {
            
            
            switch (entry.type)
            {
                case data_ActorType::DUMMY:
                    return actorLoadSpecPrototype<Actor,data_Actor>(entry);
                case data_ActorType::PLAYER:
                    return actorLoadSpecPrototype<Character,data_Character,true>(entry);
                case data_ActorType::ITEM_ACTOR:
                    return actorLoadGeneric<ItemActor,data_ItemActor>(entry);
                case data_ActorType::PHYSICS:
                    return actorLoadSpecPrototype<RigidbodyActor,data_RigidbodyActor>(entry);
                 case data_ActorType::CONSTRUCTION:
                    try {
                        data_Construction* data = cista::deserialize<data_Construction>(entry.data);
                        auto construction = Construction::makeInstanceFromSave(*data,registry.getMaterial(Loader::DEFAULT_CONSTRUCTION_MATERIAL_KEY),*this);
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

        Recipe* getRecipePrototype(string name) {
            return registry.getRecipe(name);
        }

};