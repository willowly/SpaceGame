#pragma once
#include "interface/interface.hpp"
#include "widget.hpp"

class Character;

class MenuObject {
    public:
        virtual void drawMenu(DrawContext context,World& world,Character& user) = 0;

        virtual ~MenuObject() {};
};