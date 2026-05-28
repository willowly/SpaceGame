#pragma once

#include "item-stack.hpp"
#include "engine/object.hpp"


struct Recipe : public Object {

    string name;
    string category;
    ItemStack result;
    std::vector<ItemStack> ingredients;
    float time = 0.5f;

    Recipe() {
        
    }

    Recipe(ItemStack result) : result(result) {
        
    }

    string getTypeName() override {
        return "recipe";
    }
};