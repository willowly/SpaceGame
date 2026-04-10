#pragma once
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include "actor/actor.hpp"
#include "block/block.hpp"
#include "item/item.hpp"
#include "helper/sprite.hpp"
#include "interface/widget.hpp"
#include "actor/components/particle-effect.hpp"
#include "interface/font.hpp"
#include <map>
#include <memory>

#include "engine/debug.hpp"
#include <algorithm>

using std::map,std::unique_ptr;

class Registry {

    map<string,Mesh<Vertex>> models;
    map<string,TextureID> textures;
    map<string,Sprite> sprites;
    map<string,Material> materials;
    map<string,Recipe> recipes;

    map<string,unique_ptr<Item>> item;
    
    map<string,unique_ptr<Actor>> actors;
    map<string,unique_ptr<Block>> blocks;
    map<string,unique_ptr<Item>> items;
    map<string,unique_ptr<Widget>> widgets;

    map<string,ParticleEffect> particleEffects;
    
    TextureID errorTexture = 0; 

    public:

        bool hasModel(string name) {
            return models.contains(name);
        }

        bool hasTexture(string name) {
            return textures.contains(name);
        }

        bool hasSprite(string name) {
            return sprites.contains(name);
        }

        bool hasMaterial(string name) {
            return materials.contains(name);
        }

        bool hasActor(string name) {
            return actors.contains(name);
        }

        bool hasBlock(string name) {
            return blocks.contains(name);
        }

        bool hasItem(string name) {
            return items.contains(name);
        }

        bool hasRecipe(string name) {
            return recipes.contains(name);
        }

        bool hasWidget(string name) {
            return widgets.contains(name);
        }

        bool hasParticleEffect(string name) {
            return particleEffects.contains(name);
        }

        Mesh<Vertex>* getModel(string name) {
            if(models.contains(name)) {
                return &models.at(name);
            } else {
                Debug::warn("no model called " + name + "\"");
            }
            return nullptr;
        }
        TextureID getTexture(string name) {
            if(textures.contains(name)) {
                return textures.at(name);
            } else {
                Debug::warn("no texture called " + name + "\"");
            }
            return errorTexture;
        }
        Sprite getSprite(string name) {
            if(sprites.contains(name)) {
                return sprites.at(name);
            } else {
                if(textures.contains(name)) {
                    return Sprite(textures.at(name));
                }
                Debug::warn("no sprite called " + name + "\"");
            }
            return Sprite(errorTexture);
        }
        Material getMaterial(string name) {
            if(materials.contains(name)) {
                return materials.at(name);
            } else {
                Debug::warn("no material called \"" + name + "\"");
            }
            return Material::none;
        }
        Actor* getActor(string name) {
            if(actors.contains(name)) {
                return actors.at(name).get();
            } else {
                Debug::warn("no actor prototype called \"" + name + "\"");
            }
            return nullptr;
        }

        Recipe* getRecipe(string name) {
            if(recipes.contains(name)) {
                return &recipes.at(name);
            } else {
                Debug::warn("no recipe called \"" + name + "\"");
            }
            return nullptr;
        }

        void addRecipesToVector(std::vector<Recipe*>& recipeList,string category,int maxIngredients) {
            for(auto& pair : recipes) {
                Recipe* recipe = &pair.second;
                if(recipe->category == category && recipe->ingredients.size() <= maxIngredients) {
                    recipeList.push_back(recipe);
                }
            }
        }

        template<typename T>
        T* getActor(string name) {
            Actor* actor = getActor(name);
            if(actor == nullptr) {
                return nullptr;
            }
            T* typedActor = dynamic_cast<T*>(actor);
            if(typedActor == nullptr) {
                return nullptr;
            }
            return typedActor;
        }

        Block* getBlock(string name) {
            if(blocks.contains(name)) {
                return blocks.at(name).get();
            } else {
                Debug::warn("no block prototype called \"" + name + "\"");
            }
            return nullptr;
        }

        Item* getItem(string name) {
            if(items.contains(name)) {
                return items.at(name).get();
            } else {
                Debug::warn("no item prototype called \"" + name + "\"");
            }
            return nullptr;
        }

        Widget* getWidget(string name) {
            if(widgets.contains(name)) {
                return widgets.at(name).get();
            } else {
                Debug::warn("no widget called \"" + name + "\"");
            }
            return nullptr;
        }

        template<typename T>
        T* getWidget(string name) {
            Widget* widget = getWidget(name);
            if(widget == nullptr) return nullptr;

            T* typedWidget = dynamic_cast<T*>(widget);
            if(typedWidget != nullptr) {
                return typedWidget;
            } else {
                Debug::warn("widget \"" + name + "\" not of type " + typeid(T).name());
                return nullptr;
            }
            
        }

        ParticleEffect* getParticleEffect(string name) {
            if(particleEffects.contains(name)) {
                return &particleEffects.at(name);
            } else {
                Debug::warn("no effect called \"" + name + "\"");
            }
            return nullptr;
        }

        Mesh<Vertex>* addModel(string name) {
            models.emplace(name,Mesh<Vertex>());
            return &models.at(name);
        }

        void addTexture(string name,TextureID texture) {
            textures.emplace(name,texture);
        }
        void addSprite(string name,Sprite sprite) {
            sprites.emplace(name,sprite);
        }
        void addRecipe(string name,Recipe recipe) {
            recipe.name = name;
            recipes.emplace(name,recipe);
        }
        void addMaterial(string name,Material material) {
            materials.emplace(name,material);
        }

        template <typename T>
        T* addActor(string name) {
            auto actor = T::makeDefaultPrototype();
            actor->name = name;
            actors.emplace(name,std::move(actor));
            return dynamic_cast<T*>(actors.at(name).get());
        }

        template <typename T>
        T* addBlock(string name) {
            auto block = std::make_unique<T>();
            block->name = name;
            blocks.emplace(name,std::move(block));
            return dynamic_cast<T*>(blocks.at(name).get());
        }

        template <typename T>
        T* addItem(string name) {
            auto item = std::make_unique<T>();
            item->name = name;
            items.emplace(name,std::move(item));
            return dynamic_cast<T*>(items.at(name).get());
        }

        template <typename T>
        T* addWidget(string name) {
            widgets.emplace(name,std::make_unique<T>());
            return dynamic_cast<T*>(widgets.at(name).get());
        }

        void addParticleEffect(string name,ParticleEffect effect) {
            particleEffects.emplace(name,effect);
        }

        VkPipeline litShader;
        VkPipeline textShader;
        VkPipeline uiShader;

        Font font;

};