#pragma once
#include <engine/world.hpp>
#include "engine/input.hpp"
#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"

using std::string;

struct ItemStack;

class Character;

class Item {
    public:
        

        Item() {

        }

        Item(string name) : name(name) {

        }

        string name;
        Sprite defaultSprite;

        virtual ~Item() {}

        virtual Sprite getIcon() {
            return defaultSprite;
        }



        virtual void equip(Character& user) {

        }

        virtual void unequip(Character& user) {

        }

        virtual void processInput(Input& input) {

        }

        virtual void step(World* world,Character& user,ItemStack& stack,float dt) {

        }

        virtual void addRenderables(Vulkan* vulkan,Character& user,float dt) {
            
        }
};