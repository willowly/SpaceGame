#pragma once
#include "interface/interface.hpp"
#include "actor/character.hpp"

class IHasMenu {
    public:
        virtual void drawMenu(DrawContext context,Character& user) = 0;
};