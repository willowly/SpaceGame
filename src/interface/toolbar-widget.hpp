#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>

class ToolbarWidget {
    
    public:
        Texture* solidTexture;
        Character* player;

        void render(Interface& interface) {

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            interface.drawRect(glm::vec2(0,40),glm::vec2(450,50),glm::vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            for (int i = 0; i < 9; i++)
            {
                interface.drawRect(glm::vec2((i*50)-200,40),glm::vec2(40,40),glm::vec2(0.5,1),player->selectedTool == i ? selected : unselected,solidTexture);

                Tool* tool = player->toolbar[i];
                if(tool != nullptr) {
                    interface.drawRect(glm::vec2((i*50)-200,40),glm::vec2(40,40),glm::vec2(0.5,1),Color(1,1,1),tool->icon);
                }
            }
            
        }
};