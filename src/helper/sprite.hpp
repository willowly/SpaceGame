#pragma once
#include "graphics/vulkan.hpp"
#include "rect.hpp"

struct Sprite {
    TextureID texture;
    Rect rect = Rect::unitSquare;

    Sprite() {}
    Sprite(TextureID texture,Rect rect) : texture(texture),rect(rect) {}
    Sprite(TextureID texture) : texture(texture),rect(Rect::unitSquare) {}


};