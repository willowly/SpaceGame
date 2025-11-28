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

        void setInt(int index,int value) {
            if(ints.size() <= index) {
                ints.resize(index+1);
            }
            ints[index] = value;
        }

        void setFloat(int index,float value) {
            if(floats.size() <= index) {
                floats.resize(index+1);
            }
            floats[index] = value;
        }

        void setString(int index,string value) {
            if(strings.size() <= index) {
                strings.resize(index+1);
            }
            strings[index] = value;
        }


        int getInt(int index,int defaultValue = 0) {
            if(ints.size() <= index) {
                return defaultValue;
            }
            return ints[index];
        }
        float getFloat(int index,float defaultValue = 0) {
            if(floats.size() <= index) {
                return defaultValue;
            }
            return floats[index];
        }
        string getString(int index,string defaultValue = "") {
            if(strings.size() <= index) {
                return defaultValue;
            }
            return strings[index];
        }
};