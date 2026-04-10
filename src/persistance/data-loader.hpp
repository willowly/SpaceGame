#pragma once
#include "data-generic.hpp"
#include "actor/data-actor.hpp"
#include "item/data-item-stack.hpp"
#include "graphics/material.hpp"
#include <string>


class Actor;
class Item;
class Block;

using std::string;

class DataLoader {

    public:
        virtual std::unique_ptr<Actor> loadActor(data_ActorEntry entry) = 0;

        virtual Item* getItemPrototype(string name) = 0;

        virtual Block* getBlockPrototype(string name) = 0;

        virtual Actor* getActorPrototype(string name) = 0;

        template <typename T>
        T * getActorPrototype(string name) {
            Actor* actor = getActorPrototype(name);
            if(actor == nullptr) {
                return nullptr;
            }
            T* typedActor = dynamic_cast<T*>(actor);
            if(typedActor == nullptr) {
                return nullptr;
            }
            return typedActor;
        }
};