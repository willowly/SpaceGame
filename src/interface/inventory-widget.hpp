#pragma once

#include <interface/interface.hpp>
#include <actor/character.hpp>

class InventoryWidget {
    
    public:
        TextureID solidTexture;
        Character* player;

        void draw(Interface& interface,Vulkan& vulkan) {

            auto unselected = Color(0.2,0.2,0.2);
            auto selected = Color(0.2,0.3,0.7);
            //interface.drawRect(vulkan,glm::vec2(0,-3),glm::vec2(101,12),glm::vec2(0.5,1),vec2(0.5,1),Color(0.5,0.5,0.5),solidTexture);
            int i = 0;
            int row = 0;

            interface.drawRect(vulkan,glm::vec2(0,0),glm::vec2(10,10),glm::vec2(0,0),vec2(0,0),unselected,solidTexture);
            // int column = 0;
            // int columnMax = 5;
            for (auto& stack : player->inventory)
            {
                interface.drawRect(vulkan,glm::vec2(0 + (i*12),0),glm::vec2(10,10),glm::vec2(0,0),vec2(0,0),unselected,solidTexture);
                interface.drawRect(vulkan,glm::vec2(0 + (i*12),0),glm::vec2(10,10),glm::vec2(0,0),vec2(0,0),unselected,stack.item.getIcon());
                i++;
                
            }
            
        }
};