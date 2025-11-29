#pragma once

#include "item/item-stack.hpp"
#include "item/recipe.hpp"


class Inventory {

    std::vector<ItemStack> items;

    public:

        std::vector<ItemStack*> getItems() {
            std::vector<ItemStack*> list;
            for (auto& stack : items)
            {
                if(stack.item != nullptr && stack.amount > 0) {
                    list.push_back(&stack);
                }
            }
            return list;
            
        }


        void give(ItemStack stack) {
            give(stack.item,stack.amount);
        }

        void give(Item* item,float amount) {
            auto stack = getStack(item);
            if(stack != nullptr) {
                stack->amount += amount;
            } else {
                items.push_back(ItemStack{item,amount});
            }
            
        }

        float take(ItemStack stack) {
            return take(stack.item,stack.amount);
        }
        //returns amount actually taken
        float take(Item* item,float amount) {
            auto stack = getStack(item);
            if(stack != nullptr) {
                if(stack->amount <= amount) {
                    float oldAmount = stack->amount;
                    stack->amount = 0;
                    return oldAmount;
                } else {
                    stack->amount -= amount;
                    return amount;
                }
            }
            return 0;
            
        }

        ItemStack* getStack(Item* item) {
            for (auto stack : getItems())
            {
                if(stack->amount > 0 && stack->item == item) {
                    return stack;
                }
            }
            return nullptr;
            
        }

        ItemStack* getStackIncludeEmpty(Item* item) {
            for (auto stack : getItems())
            {
                if(stack->item == item) {
                    return stack;
                }
            }
            return nullptr;
            
        }
        
        bool has(ItemStack stack) {
            return has(stack.item,stack.amount);
        }

        bool has(Item* item,float amount) {
            auto stack = getStack(item);
            if(stack == nullptr) {
                return false;
            }
            if(stack->amount < amount) {
                return false;
            }
            return true;
        }

        bool hasIngredients(Recipe& recipe) {
            for (auto& ingredient : recipe.ingredients)
            {
                if(!has(ingredient)) {
                    return false;
                }
            }
            return true;
        }

        bool tryCraft(Recipe& recipe) {
            if(!hasIngredients(recipe)) return false;
            for (auto& ingredient : recipe.ingredients)
            {
                take(ingredient);
            }
            give(recipe.result);

            return true;
        }
};