#pragma once

#include "item.hpp"

struct ItemStack {
    Item* item;
    float amount;
    ItemStack(Item* item,float amount) : item(item), amount(amount) {}
};