#pragma once
#include <engine/world.hpp>
#include "engine/input.hpp"
#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"

using std::string;

struct ItemStack;

class Character;

// just the durability bar for now, but maybe there will be more idk
struct ItemDisplayData {
    bool bar = false;
    float barPercent = 0;
    ItemDisplayData() {}
    ItemDisplayData(float barPercent) : bar(true), barPercent(barPercent) {}
};

class Item {
    public:
        

        Item() {

        }

        Item(string name) : name(name) {

        }

        string name;
        Sprite defaultSprite;
        Mesh<Vertex>* defaultModel = nullptr;
        Material defaultMaterial = Material::none;
        float defaultModelScale = 0.3f;

        virtual ~Item() {}

        virtual Sprite getIcon() {
            return defaultSprite;
        }

        virtual ItemDisplayData getItemDisplay(ItemStack& stack) {
            return ItemDisplayData();
        }

        virtual void addRenderables(Vulkan* vulkan,ItemStack& stack,vec3 position,quat rotation) {
            if(defaultModel != nullptr) {
                defaultModel->addToRender(vulkan,defaultMaterial,position,rotation,vec3(defaultModelScale));
            }
        }

        virtual void equip(Character& user) {

        }

        virtual void unequip(Character& user) {

        }

        virtual void processInput(Input& input) {

        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

        }

        virtual void addRenderablesHeld(Vulkan* vulkan,Character& user,float dt) {
            
        }
};