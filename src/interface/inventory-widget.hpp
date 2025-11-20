#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"
#include "interface/font.hpp"
#include "helper/sprite.hpp"

class InventoryWidget {
    
    public:
        Sprite solid;
        Character* player;
        Font* font;
        int number;

        void draw(Interface& interface,Vulkan& vulkan) {

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);

            interface.drawRect(vulkan,Rect(0,0,10,10),vec2(0,0),unselected,solid);

            float width = 3;
            float ratio = 1.5;
            float padding = 0.2f;
            number += 1;
            


            int i = 0;
            // int column = 0;
            // int columnMax = 5;
            for (auto& stack : player->inventory)
            {
                auto rect = Rect(0 + (i*12),0,10,10);
                interface.drawRect(vulkan,rect,vec2(0,0),unselected,solid);
                interface.drawRect(vulkan,rect,vec2(0,0),unselected,stack.item.getIcon().texture);
                string str = std::to_string(stack.amount);

                vec2 position = rect.position;
                for(char c : str) {
                    interface.drawRect(vulkan,Rect(position,vec2(width,width*ratio)),vec2(0,0),Color::white,font->getSprite(c));
                    position.x += width + padding;
                }
                
                i++;
                
            }
            
        }
};