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
    
    Window *window;

    Vulkan *vulkan;

    Loader loader;

    sol::state lua;

    Registry registry;

    public:
        TestApp() {
            window = new Window("Test App", 1000, 800);
            vulkan = new Vulkan("Test App Renderer", window);
        }
        void run() {

            Camera camera;
            camera.position = vec3(0,0,10);

            Mesh<Vertex> cube;
            cube.loadFromFile("assets/models/cube.obj");
            cube.createBuffers(vulkan);

            TextureID texture = vulkan->loadTextureFile("assets/textures/terrain/rock.png");

            Material material = vulkan->createMaterial<LitMaterialData,Vertex>("lit",LitMaterialData(0));
            float lastTime = 0;
            
            while(!window->shouldClose()) {

                glm::ivec2 frameSize = window->getFrameBufferSize();

                // Get time
                float dt = (float)glfwGetTime() - lastTime;
                lastTime = glfwGetTime();

                camera.setAspect(frameSize.x,frameSize.y);
                cube.addToRender(vulkan,material,mat4(1.0f));

                cube.addToRender(vulkan,material,glm::translate(glm::mat4(1.0f),vec3(0,1,0)));
                
                vulkan->render(camera);
                vulkan->clearObjects();


                window->pollInput();
            }

        }

};