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
    TerrainLoader* loader = nullptr;

    public:
        DataLoaderImpl(Registry& registry,TerrainLoader* loader = nullptr) : registry(registry), loader(loader) {}

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
        
            
            if(entry.type == "dummy") return actorLoadSpecPrototype<Actor,data_Actor>(entry);
            if(entry.type == "character") return actorLoadSpecPrototype<Character,data_Character,true>(entry);
            if(entry.type == "item_actor") return actorLoadGeneric<ItemActor,data_ItemActor>(entry);
            if(entry.type == "physics") return actorLoadSpecPrototype<RigidbodyActor,data_RigidbodyActor>(entry);
            if(entry.type == "construction") {
                try {
                    data_Construction* data = cista::deserialize<data_Construction>(entry.data);
                    auto construction = Construction::makeInstanceFromSave(*data,registry.getMaterial(Loader::DEFAULT_CONSTRUCTION_MATERIAL_KEY),*this);
                    return construction;
                } catch(std::runtime_error e) {
                    std::cout << "Failed to deserialize actor data from buffer: "<< e.what() << std::endl;
                }
            }
            if(entry.type == "terrain") {
                try {
                    data_Terrain* data = cista::deserialize<data_Terrain>(entry.data);
                    auto terrain = Terrain::makeInstanceFromSave(*data,registry.getMaterial(Loader::DEFAULT_TERRAIN_MATERIAL_KEY),*this);
                    if(loader != nullptr) {
                        loader->addTerrain(terrain->id);
                    }
                    return terrain;
                } catch(std::runtime_error e) {
                    std::cout << "Failed to deserialize actor data from buffer: "<< e.what() << std::endl;
                }
            }
            Debug::warn("Invalid Actor in save file: " + (string)entry.type);
            return nullptr;
        }

        Actor* getActorPrototype(string name) override {
            return registry.getActor(name);
        }

        Item* getItemPrototype(string name) override {
            return registry.getItem(name);
        }

        Block* getBlockPrototype(string name) override {
            return registry.getBlock(name);
        }

        Recipe* getRecipePrototype(string name) override {
            return registry.getRecipe(name);
        }

        TerrainSettings* getTerrainSettings(string name) override {
            return registry.getPtr<TerrainSettings>(name);
        }

};