#pragma once
#include <engine/world.hpp>
#include "engine/input.hpp"
#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"

using std::string;

class Character;

class Item {
    public:
        

        Item() {

        }

        Item(string name) : name(name) {

        }

        string name;

        virtual ~Item() {}

        virtual Sprite getIcon() = 0;


        virtual void equip(Character& user) {

        }

        virtual void unequip(Character& user) {

        }

        virtual void processInput(Input& input) {

        }

        virtual void step(World* world,Character& user,float dt) {

        }

        virtual void addRenderables(Vulkan* vulkan,Character& user,float dt) {
            
        }
};