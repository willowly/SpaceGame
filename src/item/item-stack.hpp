#pragma once

#include "item.hpp"
#include "helper/generic-storage.hpp"

#include "persistance/item/data-item-stack.hpp"
#include "persistance/data-loader.hpp"

struct ItemStack {
    Item* item = nullptr; 
    int amount = 0;
    GenericStorage storage;
    ItemStack(Item* item,int amount,GenericStorage storage) : item(item), amount(amount), storage(storage) {}
    ItemStack(Item* item,int amount) : item(item), amount(amount) {}
    ItemStack() {}

    bool has(ItemStack stack) {
        return item == stack.item && amount >= stack.amount;
    }

    // false must guarentee item is non-null and valid
    bool isEmpty() {
        return item == nullptr || amount == 0;
    }

    void clear() {
        item = nullptr;
        storage.clear();
    }

    bool canInsert(ItemStack stack) {
        if(isEmpty()) {
            return true;
        }
        if(item == stack.item) {
            return true;
        }
        return false;
    }

    bool tryInsert(ItemStack stack) {
        if(isEmpty()) {
            *this = stack;
            return true;
        }
        if(item == stack.item) {
            amount += stack.amount;
            return true;
        }
        return false;

    }

    int take(ItemStack stack) {
        if(item == stack.item) {
            if(amount > stack.amount) {
                amount -= stack.amount;
                return stack.amount;
            } else {
                amount = 0;
                return amount;
            }
        }
        return 0;

    }

    data_ItemStack save() {
        data_ItemStack data;
        if(item != nullptr) {
            data.item = item->name;
        }
        data.amount = amount;
        data.storage = storage.save();
        return data;
    }

    void load(const data_ItemStack data,DataLoader& loader) {
        item = loader.getItemPrototype((string)data.item);
        amount = data.amount;
        storage.load(data.storage);
    }
};