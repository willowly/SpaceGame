#pragma once

#include "persistance/data-generic.hpp"
#include "persistance/data-generic-storage.hpp"
#include "persistance/item/data-item-stack.hpp"
#include "cista.h"

enum class data_ResourcePointerType {
    NONE,
    ITEM,
    RECIPE,
};

struct data_ResourcePointer {
    data_ResourcePointerType type;
    cista::raw::string name;
};

struct data_BlockStorage {
    
    data_GenericStorage genericStorage;
    cista::raw::vector<data_ItemStack> stacks;
    cista::raw::vector<data_ResourcePointer> pointers;
};