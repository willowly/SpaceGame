#pragma once

#include "cista.h"
#include "data-generic-storage.hpp"
#include "block/block-state.hpp"

struct data_Block {
    cista::raw::string block;
    uint64_t state;
};