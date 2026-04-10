#pragma once

#include "persistance/data-generic.hpp"
#include "persistance/data-generic-storage.hpp"
#include "cista.h"

struct data_ItemStack {
    cista::raw::string item;
    int amount;
    data_GenericStorage storage;
};