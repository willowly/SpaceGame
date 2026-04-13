#pragma once

#include "item-stack.hpp"


struct Recipe {

    string name;
    string category;
    ItemStack result;
    std::vector<ItemStack> ingredients;
    float time = 0.5f;

    Recipe() {
        
    }

    Recipe(ItemStack result) : result(result) {
        
    }
};