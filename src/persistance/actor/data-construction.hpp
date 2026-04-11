#pragma once
#include "persistance/block/data-block-storage.hpp"
#include "persistance/item/data-item-stack.hpp"
#include "persistance/actor/data-actor.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"
#include "persistance/actor/component/data-inventory.hpp"
#include "persistance/block/data-block.hpp"
#include "cista.h"

struct data_BlockStoragePair {
    data_ivec3 position;
    data_BlockStorage storage;
};

struct data_Construction {
    data_Actor actor;
    data_Rigidbody body;
    cista::raw::vector<data_Block> blocks;
    data_ivec3 boundsMin;
    data_ivec3 boundsMax;
    cista::raw::vector<data_BlockStoragePair> storages;
    cista::raw::vector<data_ivec3> stepCallbacks;
    bool isStatic;
};