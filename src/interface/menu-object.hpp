#pragma once
#include "interface/interface.hpp"

class Character;

class MenuObject {
    public:
        virtual void drawMenu(DrawContext context,Character& user) = 0;

        virtual ~MenuObject() {};
};