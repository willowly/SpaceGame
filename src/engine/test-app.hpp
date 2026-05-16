#pragma once

#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "graphics/vulkan.hpp"
#include "engine/window.hpp"
#include "imgui/imgui.h"
#include "graphics/skybox.hpp"
// #include <tracy/Tracy.hpp>
#include "cista.h"

#include "engine/loader.hpp"

#include "persistance/data-loader-impl.hpp"

#include "actor/character.hpp"

#include <string>

#include <iostream>

#include <format>
#include <functional>

class TestApp {

    Window* window;
    Vulkan* vulkan;
    
    void setup() {
        
    }

    void loop() {

    }

    public:

        TestApp() {
            window = new Window("Test app",100,100);
            vulkan = new Vulkan("Whatever",window);
        }

        ~TestApp() {
            delete vulkan;
            delete window;
        }

        void run() {

            
            while(!window->shouldClose()) {

                window->pollInput();
                
                // do stuff
            }
        }

};