#pragma once
#include "graphics/vulkan.hpp"
#include "rect.hpp"

struct Sprite {
    string name = "sprite";
    TextureID texture = 0;
    Rect rect = Rect::unitSquare;

    Sprite() {}
    Sprite(string name,TextureID texture) : name(name), texture(texture) {}
    Sprite(string name,TextureID texture,Rect rect) : name(name), texture(texture),rect(rect) {}
    Sprite(TextureID texture,Rect rect) : texture(texture),rect(rect) {}
    Sprite(TextureID texture) : texture(texture),rect(Rect::unitSquare) {}


};