#pragma once
#define VULKAN_NO_PROTOTYPES
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <format>
#include <set>
#include "helper/file-helper.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/mesh.hpp"
#include "engine/input.hpp"
#include "helper/string-helper.hpp"
#include "helper/random.hpp"
#include "graphics/camera.hpp"
#include "graphics/skybox.hpp"
#include "world.hpp"
#include "actor/actors-all.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "interface/interface.hpp"
#include "helper/clock.hpp"
#include "terrain-loader.hpp"
#include <thread>
#include <chrono>

#include "interface/actor/player-widget.hpp"
#include "interface/block/furnace-widget.hpp"

#include "item/items-all.hpp"
#include "item/recipe.hpp"

#include "physics/jolt-layers.hpp"


#include "imgui/imgui.h"
#include "persistance/data-world.hpp"
#include "persistance/save-helper.hpp"
#include "persistance/data-loader-impl.hpp"

#include "interface/debug/cheats-menu.hpp"

JPH_SUPPRESS_WARNINGS

using std::string;
using std::unique_ptr, std::shared_ptr;


class GameApplication {

    

    public:
        GameApplication(string name) : name(name) {
            
            initWindow();
            vulkan = new Vulkan(name,window);

        }

        ~GameApplication() {
            
            delete vulkan;

            glfwDestroyWindow(window);

            glfwTerminate();
        }
        string name;
        uint32_t windowWidth = 1280;
        uint32_t windowHeight = 720;
        void run() {

            setup();

            terrainLoader.start();
            

            while (!glfwWindowShouldClose(window)) {
                loop();

                // if(input.getKey(GLFW_KEY_ESCAPE)) {
                //     break;
                // }
            }

            
            terrainLoader.stop();
            vulkan->waitIdle();

        }

        
        
        private:

        std::unique_ptr<World> world = nullptr;
        
        std::vector<std::thread> chunkWorkers;

        Registry registry;

        Loader loader;
        
        sol::state lua;

        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        Input input;

        std::vector<float> frameTimes;

        std::shared_ptr<Character> player = nullptr;

        Interface interface;

        PlayerWidget playerWidget;
        ToolbarWidget toolbarWidget;
        InventoryWidget inventoryWidget;
        ItemSlotWidget clearItemSlotWidget;
        TextWidget fpsText;

        FurnaceWidget furnaceWidget;

        Font font;

        TerrainLoader terrainLoader;


        Material terrainMaterial = Material::none;
        Material terrainMaterialDebug = Material::none;


        Recipe makeAluminumPlate;
        Recipe makeFurnace;
        Recipe makeThruster;
        Recipe makeCockpit;
        Recipe makePickaxe;

        Skybox skybox;

        std::atomic<bool> closing = false;
        std::atomic<bool> chunkLoadPaused = false;

        GenerationSettings generationSettings;

        float frametimes[60];
        int currentFrameTimeIndex = 0;

        bool debugUIOpen = false;
        char consoleBuffer[1024] = ""; //temp
        

        float lastTime = 0; //tells how long its been since the last update

        static void errorCallback(int error, const char* description) {
            std::cout << std::format("GLFW Error: {}\n {}",error,description) << std::endl;
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            app->vulkan->setFrameBufferResized();
        }

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            if(action == GLFW_PRESS) {
                input.keys[key] = true;
                input.keysPressed[key] = true;
            }
            if(action == GLFW_RELEASE) {
                input.keys[key] = false;
                input.keysReleased[key] = true;
            }
        }

        static void characterCallback(GLFWwindow* window, unsigned int codepoint)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            input.textInput += (char)codepoint;
        }



        static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            #if __APPLE__
                input.currentMousePosition = vec2(xposIn,yposIn)* 2.0f; //retina bs. Doesn't actually detect retina so itll probably not work on macs that have no retina
            #else
                input.currentMousePosition = vec2(xposIn,yposIn);
            #endif
        }

        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            if(action == GLFW_PRESS) {
                input.mouseButtons[button] = true;
                input.mouseButtonsPressed[button] = true;
            }
            if(action == GLFW_RELEASE) {
                input.mouseButtons[button] = false;
                input.mouseButtonsReleased[button] = true;
            }
        }

        void initWindow();

        void spawnPlayer();

        void spawnAsteroidScene();

        void setup();

        void debugUI(float dt);

        void loop();

        

        

        
};