#pragma once
#include "interface/interface.hpp"
#include "widget.hpp"

class Character;

class MenuObject : public Widget {
    public:
        virtual void drawMenu(DrawContext context,Character& user) = 0;

        virtual ~MenuObject() {};
};