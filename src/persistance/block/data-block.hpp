#pragma once

#include "cista.h"
#include "persistance/data-generic-storage.hpp"
#include "block/block-facing.hpp"
#include "persistance/block/data-block-storage.hpp"
#include "block/block-id.hpp"

struct data_BlockData {
    BlockID id;
};

struct data_BlockPaletteEntry {
    cista::raw::string block;
    data_BlockStorage storage;
};