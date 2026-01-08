#pragma once
#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"
#include <string>

using std::string;

class Font {

    public:
        TextureID texture;
        char start;
        vec2 charSize;
        vec2 textureSize;
        string characters;


        Sprite getSprite(char c) {
            int index = characters.find(c);
            Rect rect(vec2(charSize.x * index,0),charSize);
            rect.position /= textureSize;
            rect.size /= textureSize;
            return Sprite(texture,rect);
        }



};