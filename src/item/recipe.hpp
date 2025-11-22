#pragma once

#include "item-stack.hpp"


struct Recipe {
    ItemStack result;
    std::vector<ItemStack> ingredients;

    Recipe(ItemStack result) : result(result) {
        
    }
};