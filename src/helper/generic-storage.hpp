#pragma once

#include "glm/glm.hpp"
#include <vector>
#include <string>
#include "persistance/data-generic-storage.hpp"

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

        data_GenericStorage save() {
            data_GenericStorage data;
            for(auto element : ints) {
                data.ints.push_back(element);
            }
            for(auto element : floats) {
                data.floats.push_back(element);
            }
            for(auto element : strings) {
                data.strings.push_back(element);
            }
            return data;
        }

        void load(const data_GenericStorage data) {
            ints.clear();
            for(auto element : data.ints) {
                ints.push_back(element);
            }
            floats.clear();
            for(auto element : data.floats) {
                floats.push_back(element);
            }
            strings.clear();
            for(auto element : data.strings) {
                strings.push_back((string)element);
            }
        }
};