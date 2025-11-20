#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>
#include "helper/rect.hpp"

class ToolbarWidget {
    
    public:
        Sprite solidSprite;
        Character* player;

        void draw(Interface& interface,Vulkan& vulkan) {

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            for (int i = 0; i < 9; i++)
            {
                auto rect = Rect((i*10)-50,-11,10,10);
                interface.drawRect(vulkan,rect,glm::vec2(0.5,1),player->selectedTool == i ? selected : unselected,solidSprite);

                Item* tool = player->toolbar[i];
                if(tool != nullptr) {
                    interface.drawRect(vulkan,rect,glm::vec2(0.5,1),Color::white,tool->getIcon().texture);
                }
            }
            
        }
};