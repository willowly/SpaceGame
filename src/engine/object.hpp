#pragma once

#include <string>

using std::string;

class Object {

    public:

        string name;

        virtual string getTypeName() = 0;

        virtual ~Object() {};
};