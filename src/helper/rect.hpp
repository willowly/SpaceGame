#pragma once
#include "glm/glm.hpp"

using glm::vec2;

struct Rect {
    vec2 position = {};
    vec2 size = {};

    Rect(vec2 size) : position(vec2(0)), size(size) {

    }
    Rect(vec2 position,vec2 size) : position(position), size(size) {

    }

    Rect(float x,float y,float width,float height) : position(x,y), size(width,height) {

    }

    Rect(float width,float height) : position(0.0f), size(width,height) {

    }

    vec2 bottomRight() {
        return vec2(position+size);
    }
    vec2 bottomLeft() {
        return vec2(position.x,position.y+size.y);
    }
    vec2 topRight() {
        return vec2(position.x+size.x,position.y);
    }
    vec2 topLeft() {
        return position;
    }

    // static Rect corners(vec2 topLeft,vec2 bottomRight) {

    // }

    static Rect withPivot(vec2 position,vec2 size,vec2 pivot) {
        return Rect(position-(size*pivot),size);
    }

    static Rect withPivot(vec2 size,vec2 pivot) {
        return Rect(-(size*pivot),size);
    }

    static Rect centered(vec2 size) {
        return Rect(-size/2.0f,size);
    }

    static Rect centered(float width,float height) {
        return Rect::centered(vec2(width,height));
    }

    static Rect square(vec2 position,float size) {
        return Rect(position,vec2(size));
    }

    static Rect square(float size) {
        return Rect(vec2(0.0f),vec2(size));
    }

    static Rect squareCentered(vec2 position,float size) {
        return Rect(position,vec2(size));
    }

    static Rect anchored(Rect rect,Rect parent,vec2 anchor) {
        rect.position += parent.position + parent.size * anchor;
        return rect;
    }


    static const Rect unitSquare;
};

const Rect Rect::unitSquare = Rect(0,0,1,1);