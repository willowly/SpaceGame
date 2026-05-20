#pragma once
#include "persistance/item/data-item-stack.hpp"
#include "persistance/actor/data-actor.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"
#include "cista.h"

struct data_ItemActor {
    data_Actor actor;
    data_Rigidbody body;
    data_ItemStack stack;
};