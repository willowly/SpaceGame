#define TRACY_ENABLE 1
#include <tracy/Tracy.hpp>
#include "engine/game-application.hpp"

#include "shaderc/shaderc.h"

int main() {
    
    GameApplication game("Super Space Miner 2000");

    game.run();


}
