#pragma once

#include "generic-storage.hpp"

#include "item/recipe.hpp"
#include "item/item-stack.hpp"
#include "block/block-facing.hpp"
// #include "block/block.hpp"
#include "persistance/block/data-block-storage.hpp"

#include <variant>

class Block;

typedef std::variant<std::monostate,Item*,Recipe*> ResourcePointer; //monostate to hold nullptr 


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
            auto pointer = pointers[index];
            if(std::holds_alternative<std::monostate>(pointer)) {
                return nullptr;
            }
            if(std::holds_alternative<T*>(pointer)) {
                return std::get<T*>(pointer);
            } else {
                Debug::warn("generic storage pointer type is wrong.");
                return nullptr;
            }
        }

        void clear() {
            GenericStorage::clear();
            stacks.clear();
            pointers.clear();
        }

        void setFacing(size_t index,BlockFacing facing) {
            setInt(index,static_cast<int>(facing));
        }

        BlockFacing getFacing(size_t index) {
            int i = getInt(index,(int)BlockFacing::FORWARD);
            if(i > static_cast<int>(BlockFacing::LEFT)) {
                return BlockFacing::LEFT;
            }
            return static_cast<BlockFacing>(i);
        }

        bool operator==(const BlockStorage& entry) {
            if(!GenericStorage::operator==(entry)) return false;
            if(pointers.size() != entry.pointers.size()) return false;
            for (size_t i = 0; i < pointers.size(); i++)
            {
                if(pointers.at(i) != entry.pointers.at(i)) return false; 
            }
            // if ur worried about itemstacks... you shouldn't be comparing these
            return true;
        }

        data_BlockStorage save() {
            data_BlockStorage data;
            data.genericStorage = GenericStorage::save();
            for(auto stack : stacks) {
                data.stacks.push_back(stack.save());
            }
            for(auto pointer : pointers) {
                data_ResourcePointer data_pointer;
                if (auto* p = std::get_if<Item*>(&pointer)) {
                    data_pointer.type = data_ResourcePointerType::ITEM;
                    if((*p) != nullptr) {
                        data_pointer.name = (*p)->name;
                    }
                } else if (auto* p = std::get_if<Recipe*>(&pointer)) {
                    data_pointer.type = data_ResourcePointerType::RECIPE;
                    if((*p) != nullptr) {
                        data_pointer.name = (*p)->name;
                    }
                } else { // std::monostate
                    data_pointer.type = data_ResourcePointerType::NONE;
                }
                data.pointers.push_back(data_pointer);
            }
            return data;
        }

        void load(const data_BlockStorage data,DataLoader& loader) {
            GenericStorage::load(data.genericStorage);
            stacks.clear();
            for(auto data_stack : data.stacks) {
                ItemStack stack;
                stack.load(data_stack,loader);
                stacks.push_back(stack);
            }
            for(auto data_pointer : data.pointers) {
                ResourcePointer pointer;
                if(data_pointer.name != "") {
                    switch (data_pointer.type) {
                        case data_ResourcePointerType::NONE:
                            break;
                        case data_ResourcePointerType::ITEM:
                            pointer = loader.getItemPrototype((string)data_pointer.name);
                            break;
                        case data_ResourcePointerType::RECIPE:
                            pointer = loader.getRecipePrototype((string)data_pointer.name);
                            break;

                    }
                }
                pointers.push_back(pointer);
            }
        }


};