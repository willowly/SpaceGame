#pragma once

#include "generic-storage.hpp"

#include "item/recipe.hpp"
#include "item/item-stack.hpp"
#include "block/block.hpp"

typedef std::variant<Item*,Recipe*,Block*> ResourcePointer;


class BlockStorage : public GenericStorage {

    std::vector<ItemStack> stacks;
    std::vector<ResourcePointer> pointers;
    public:
        void setStack(size_t index,ItemStack value) {
            if(stacks.size() <= index) {
                stacks.resize(index+1);
            }
            stacks[index] = value;
        }
        void clearStack(size_t index) {
            setStack(index,ItemStack(nullptr,0));
        }

        template<typename T>
        void setPointer(size_t index,T* value) {
            if(pointers.size() <= index) {
                pointers.resize(index+1);
            }
            pointers[index] = value;
        }
        ItemStack getStack(size_t index) {
            if(stacks.size() <= index) {
                return ItemStack(nullptr,0);
            }
            return stacks[index];
        }

        template<typename T>
        T* getPointer(size_t index) {
            if(pointers.size() <= index) {
                return nullptr;
            }
            if(std::holds_alternative<T*>(pointers[index])) {
                return std::get<T*>(pointers[index]);
            } else {
                Debug::warn("generic storage pointer type is wrong.");
                return nullptr;
            }
        }

        void clear() {
            BlockStorage::clear();
            stacks.clear();
            pointers.clear();
        }


};