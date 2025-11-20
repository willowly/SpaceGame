#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"

class Font {

    public:
        TextureID texture;
        char start;
        vec2 charSize;
        vec2 textureSize;


        Sprite getSprite(char c) {
            int index = c - start;
            Rect rect(vec2(charSize.x * index,0),charSize);
            rect.position /= textureSize;
            rect.size /= textureSize;
            return Sprite(texture,rect);
        }



};