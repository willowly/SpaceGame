#pragma once

#include "item.hpp"
#include "helper/generic-storage.hpp"

struct ItemStack {
    Item* item = nullptr; 
    float amount = 0;
    GenericStorage storage;
    ItemStack(Item* item,float amount,GenericStorage storage) : item(item), amount(amount), storage(storage) {}
    ItemStack(Item* item,float amount) : item(item), amount(amount) {}
    ItemStack() {}

    bool has(ItemStack stack) {
        return item == stack.item && amount >= stack.amount;
    }

    bool empty() {
        return item == nullptr || amount == 0;
    }

    void clear() {
        item = nullptr;
        storage.clear();
    }

    bool tryInsert(ItemStack stack) {
        if(empty()) {
            *this = stack;
            return true;
        }
        if(item == stack.item) {
            amount += stack.amount;
            return true;
        }
        return false;

    }
};