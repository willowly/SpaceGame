#pragma once

#include "persistance/data-generic.hpp"
#include "persistance/item/data-item-stack.hpp"
#include "cista.h"

struct data_Inventory {
    cista::raw::vector<data_ItemStack> itemStacks;
};