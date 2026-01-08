#pragma once
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include "actor/actor.hpp"
#include "block/block.hpp"
#include "item/item.hpp"
#include "helper/sprite.hpp"
#include "interface/widget.hpp"
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

    map<string,unique_ptr<Item>> item;
    
    map<string,unique_ptr<Actor>> actors;
    map<string,unique_ptr<Block>> blocks;
    map<string,unique_ptr<Item>> items;
    map<string,unique_ptr<Widget>> widgets;
    
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

        bool hasWidget(string name) {
            return widgets.contains(name);
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
        void addMaterial(string name,Material material) {
            materials.emplace(name,material);
        }

        template <typename T>
        T* addActor(string name) {
            actors.emplace(name,T::makeDefaultPrototype());
            return dynamic_cast<T*>(actors.at(name).get());
        }

        template <typename T>
        T* addBlock(string name) {
            blocks.emplace(name,std::make_unique<T>());
            return dynamic_cast<T*>(blocks.at(name).get());
        }

        template <typename T>
        T* addItem(string name) {
            items.emplace(name,std::make_unique<T>());
            return dynamic_cast<T*>(items.at(name).get());
        }

        template <typename T>
        T* addWidget(string name) {
            widgets.emplace(name,std::make_unique<T>());
            return dynamic_cast<T*>(items.at(name).get());
        }

        VkPipeline litShader;
        VkPipeline textShader;
        VkPipeline uiShader;

};