
#pragma once

#include "interface/interface.hpp"
#include "font.hpp"
#include "interface/widget.hpp"

struct TextDisplaySettings {
    vec2 pivot = {};
};

class TextWidget : public Widget {

    public:

        float height = 12;
        float ratio = 1.5;
        float spacing = 0.25f;
        Color color = Color::white;
        Font* font = nullptr;


    void draw(DrawContext context,vec2 position,string text,TextDisplaySettings settings = {}) {
        if(font == nullptr) {
            Debug::warn("no font on text widget");
            return;
        }
        float width = height/ratio;
        int textLength = text.length();
        vec2 totalSize = getSize(text);
        Rect totalRect = Rect::withPivot(position,totalSize,settings.pivot);
        Rect characterRect = Rect(totalRect.topLeft(),vec2(width,height));

        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c){ return std::toupper(c); }); // for now lol
        for(char c : text) {
            
            context.drawRect(characterRect,font->getSprite(c));
            characterRect.position.x += width + spacing;
            
        }
    }

    vec2 getSize(string text) {
        return getSize(text.size());
    }
    vec2 getSize(int length) {
        float width = height/ratio;
        return vec2((width + spacing) * length,height);
    }

    string getTypeName() override {
        return "text_widget";
    }
    
};