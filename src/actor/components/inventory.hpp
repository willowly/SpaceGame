#pragma once

#include "item/item-stack.hpp"
#include "item/recipe.hpp"
#include "persistance/actor/component/data-inventory.hpp"
#include "persistance/data-loader.hpp"
#include <ranges>

class IInventory {
    
    protected:
        virtual ItemStack& at(size_t index) = 0; //if the index is larger than the size, resize the representation
        virtual size_t size() = 0;
        virtual void clear() = 0;

    public:

        float maxWeight = 0; // 0 disables max weight

        std::vector<ItemStack*> getItems() {
            std::vector<ItemStack*> list;
            for (size_t i = 0; i < size(); i++)
            {
                auto& stack = at(i);
                if(stack.item != nullptr && stack.amount > 0) {
                    list.push_back(&stack);
                }
            }
            return list;
            
        }


        void give(Item* item,int amount) {
            give(ItemStack(item,amount));
            
        }

        void give(ItemStack newStack) {
            if(newStack.isEmpty()) return;
            auto stack = getStack(newStack.item);
            // if the stack is null, try inserting. If inserting fails, add a new item. Kinda weird syntax maybe i can make it better idk
            if(stack == nullptr || !stack->tryInsert(newStack)) {
                at(size()) = newStack; //set the last element.
            }
            
        }

        // returns the remaining items (empty if they fit)
        ItemStack tryGive(ItemStack newStack) {
            if(maxWeight > 0 && getTotalWeight() > maxWeight) return newStack; //reject it outright
            give(newStack);
            if(maxWeight == 0) return {};
            float overWeight = getTotalWeight() - maxWeight;
            if(overWeight > 0) {
                newStack.amount = take(newStack.item,ceil(overWeight / newStack.item->getWeight()));
            } else {
                newStack.amount = 0;
            }
            return newStack;
        }

        float getTotalWeight() {
            float weight = 0;
            for (size_t i = 0; i < size(); i++)
            {
                auto& stack = at(i);
                weight += stack.getWeight();
            }
            return weight;
        }

        float getSpaceLeft() {
            if(maxWeight == 0) return std::numeric_limits<int>().max();

            return maxWeight - getTotalWeight();
        }



        int take(ItemStack stack) {
            return take(stack.item,stack.amount);
        }
        //returns amount actually taken
        int take(Item* item,int amount) {
            if(item == nullptr) return 0;
            auto stack = getStack(item);
            if(stack != nullptr) {
                if(stack->amount <= amount) {
                    int oldAmount = stack->amount;
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
                if(stack->item == item) {
                    return stack;
                }
            }
            return nullptr;
            
        }

        ItemStack* getStackIncludeEmpty(Item* item) {
            for (size_t i = 0; i < size(); i++)
            {
                auto& stack = at(i);
                if(stack.item == item) {
                    return &stack;
                }
            }
            return nullptr;
            
        }
        
        bool has(ItemStack stack) {
            return has(stack.item,stack.amount);
        }

        bool has(Item* item,int amount) {
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

        data_Inventory save() {
            data_Inventory data;
            for (auto stack : getItems())
            {
                data.itemStacks.push_back(stack->save());
            }
            return data;
        }

        void load(data_Inventory data,DataLoader& loader) {
            for (size_t i = 0; i < size(); i++)
            {
                at(i) = ItemStack(); //remove all items
            }
            for (auto data_stack : data.itemStacks)
            {
                ItemStack stack;
                stack.load(data_stack,loader);
                at(size()) = stack;
            }
        }

        IInventory() {}
        virtual ~IInventory() {}
        IInventory(const IInventory& inventory) = delete;
        IInventory& operator=(const IInventory& inventory) = delete;
};

class Inventory : public IInventory {

    std::vector<ItemStack> items;

    protected:
        ItemStack& at(size_t index) override {
            if(index >= items.size()) {
                items.resize(index+1);
            }
            return items[index];
        }
        size_t size() override {
            return items.size();
        }
        void clear() override {
            items.clear();
        }
};

class BlockInventory : public IInventory {

    BlockStorage& storage;
    int startIndex = 0;
    int sizeVar = 0;

    

    protected:
        ItemStack& at(size_t index) override {
            int size = storage.getInt(sizeVar);
            if(index+1 > size) {
                storage.setInt(sizeVar,static_cast<int>(index+1));
            }
            return storage.getStack(startIndex + index);
        } 
        size_t size() override {
            return storage.getInt(sizeVar);
        }
        void clear() override {
            for (size_t i = 0; i < size(); i++)
            {
                storage.clearStack(startIndex + i);
            }
            storage.setInt(sizeVar,0);
            
        }

    public:
        BlockInventory(BlockStorage& storage,int startIndex,int sizeVar,float maxWeight = {}) : storage(storage), startIndex(startIndex), sizeVar(sizeVar), IInventory() {
            this->maxWeight = maxWeight; 
        }
};