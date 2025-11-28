#pragma once

#include "item-stack.hpp"


struct Recipe {
    ItemStack result;
    std::vector<ItemStack> ingredients;
    float time;

    Recipe(ItemStack result) : result(result) {
        
    }
};