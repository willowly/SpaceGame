#pragma once

#include "cista.h"
#include "persistance/data-generic-storage.hpp"
#include "block/block-state.hpp"
#include "persistance/block/data-block-storage.hpp"

struct data_Block {
    cista::raw::string block;
    uint64_t state;
    data_BlockStorage storage;
};