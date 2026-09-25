#pragma once
#include <glm/glm.hpp>
#include "helper/sprite.hpp"
#include "graphics/color.hpp"
#include "interface/interface.hpp"
#include "widget.hpp"

using glm::vec2;

struct PanelWidget : public Widget {
    
    
    vec2 position;
    vec2 size;
    vec2 anchor;
    vec2 pivot;
    Sprite sprite;
    Color color;

    Rect getRect(Rect parent) {
        return Rect::anchored(Rect::withPivot(position,size,pivot),parent,anchor);
    }

    Rect draw(DrawContext context) {
        return draw(context,context.getScreenSize());
    }

    Rect draw(DrawContext context,Rect parent) {
        Rect rect = getRect(parent);
        context.drawRect(rect,sprite,color);
        return rect;
    }

    string getTypeName() override {
        return "panel_widget";
    }
};