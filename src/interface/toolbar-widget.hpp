#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>

class ToolbarWidget {
    
    public:
        TextureID solidTexture;
        Character* player;

        void draw(Interface& interface,Vulkan& vulkan) {

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            for (int i = 0; i < 9; i++)
            {
                interface.drawRect(vulkan,glm::vec2((i*10)-50,-11),glm::vec2(10,10),glm::vec2(0.5,1),vec2(0,0),player->selectedTool == i ? selected : unselected,solidTexture);

                Tool* tool = player->toolbar[i];
                if(tool != nullptr) {
                    interface.drawRect(vulkan,glm::vec2((i*10)-50,-11),glm::vec2(10,10),glm::vec2(0.5,1),vec2(0,0),Color::white,tool->icon);
                }
            }
            
        }
};