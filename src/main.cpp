#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "engine/game-application.hpp"
#include "networking/net-test-app.hpp"
#include "engine/test-app.hpp"
#include <iostream>
#include <print>

//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using std::string;


int main() {

    try {
        
        TestApp app;

        app.run();

    } catch (std::exception error) {
        std::cout << error.what() << std::endl;
    } catch (...) {
        std::cout << "Caught a non-std::exception object" << std::endl;
    }
}
