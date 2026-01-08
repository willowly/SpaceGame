#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <string>

using std::string;


class GenericStorage {
    std::vector<int> ints;
    std::vector<float> floats;
    std::vector<string> strings;


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

        virtual void clear() {
            ints.clear();
            floats.clear();
            strings.clear();
            
        }
};