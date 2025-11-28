#pragma once

#include "item.hpp"

struct ItemStack {
    Item* item = nullptr; 
    float amount = 0;
    ItemStack(Item* item,float amount) : item(item), amount(amount) {}
    ItemStack() {}

    bool has(ItemStack stack) {
        return item == stack.item && amount >= stack.amount;
    }
};