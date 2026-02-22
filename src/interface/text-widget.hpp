
#pragma once

#include "interface/interface.hpp"
#include "font.hpp"
#include "interface/widget.hpp"

class TextWidget : public Widget {

    public:

        float height = 3;
        float ratio = 1.5;
        float spacing = 0.25f;
        Color color = Color::white;
        Font* font = nullptr;


    void draw(DrawContext context,vec2 position,string text) {
        if(font == nullptr) {
            Debug::warn("no font on text widget");
            return;
        }
        float width = height/ratio;
        Rect characterRect = Rect(position,vec2(width,height));

        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::toupper(c); }); // for now lol
        for(char c : text) {
            
            context.drawRect(characterRect,Color::white,font->getSprite(c));
            characterRect.position.x += width + spacing;
            
        }
    }

    vec2 getSize(string text) {
        float width = height/ratio;
        return vec2((width + spacing) * text.size(),height);
    }
    
};