#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "engine/game-application.hpp"
//#include <tracy/Tracy.hpp>
#include "cista.h"

#include <iostream>



int main() {

    std::cout << "Current loader path is: " << std::filesystem::current_path() << '\n';
    
    GameApplication game("Super Space Miner 2000");

    game.run();

   



    

}
