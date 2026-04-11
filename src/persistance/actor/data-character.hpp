#pragma once
#include "persistance/item/data-item-stack.hpp"
#include "persistance/actor/data-actor.hpp"
#include "persistance/actor/component/data-rigidbody.hpp"
#include "persistance/actor/component/data-inventory.hpp"
#include "cista.h"

struct data_Character {
    data_Actor actor;
    data_Rigidbody body;
    int selectedTool = 0;
    float lookPitch = 0;
    bool thirdPerson = false;

    cista::raw::vector<data_ItemStack> toolbar;
    data_Inventory inventory;
};