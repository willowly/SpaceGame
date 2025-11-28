#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <string>

using std::string;

#include "item/recipe.hpp"
#include "item/item-stack.hpp"
#include "block/block.hpp"

typedef std::variant<Item*,Recipe*,Block*> ResourcePointer;


class GenericStorage {
    std::vector<int> ints;
    std::vector<float> floats;
    std::vector<string> strings;
    std::vector<ItemStack> stacks;
    std::vector<ResourcePointer> pointers;


    public:

        void setInt(size_t index,int value) {
            if(ints.size() <= index) {
                ints.resize(index+1);
            }
            ints[index] = value;
        }

        void setFloat(size_t index,float value) {
            if(floats.size() <= index) {
                floats.resize(index+1);
            }
            floats[index] = value;
        }

        void setString(size_t index,string value) {
            if(strings.size() <= index) {
                strings.resize(index+1);
            }
            strings[index] = value;
        }


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


        int getInt(size_t index,int defaultValue = 0) {
            if(ints.size() <= index) {
                return defaultValue;
            }
            return ints[index];
        }
        float getFloat(size_t index,float defaultValue = 0) {
            if(floats.size() <= index) {
                return defaultValue;
            }
            return floats[index];
        }
        string getString(size_t index,string defaultValue = "") {
            if(strings.size() <= index) {
                return defaultValue;
            }
            return strings[index];
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
};