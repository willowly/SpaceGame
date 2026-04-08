#define TRACY_ENABLE 1
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "engine/game-application.hpp"
//#include <tracy/Tracy.hpp>
#include <windows.h> 

#include "shaderc/shaderc.h"

int main() {

    std::cout << "Current loader path is: " << std::filesystem::current_path() << '\n';
    
    GameApplication game("Super Space Miner 2000");

    game.run();

}
