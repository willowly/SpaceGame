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
#include "helper/random-helper.hpp"
#include "graphics/camera.hpp"
#include "graphics/skybox.hpp"
#include "world.hpp"
#include "actor/actors-all.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "interface/interface.hpp"
#include "helper/clock.hpp"
#include <thread>
#include <chrono>

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#include "interface/actor/player-widget.hpp"
#include "interface/block/furnace-widget.hpp"

#include "item/items-all.hpp"
#include "item/recipe.hpp"

#include "physics/jolt-layers.hpp"



JPH_SUPPRESS_WARNINGS

using std::string;


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

            std::cout << "main thread id:" << std::this_thread::get_id() << std::endl;

            setup();

            

            while (!glfwWindowShouldClose(window)) {
                loop();
            }

            
            
            closing = true;
            //thread.join();
            vulkan->waitIdle();

        }

        
        
        private:

        World world;
        

        Registry registry;

        Loader loader;
        
        sol::state lua;

        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        Input input;
        
        Camera camera;

        std::vector<float> frameTimes;

        Character* player;
        Terrain* terrain;

        Interface interface;

        PlayerWidget playerWidget;
        ToolbarWidget toolbarWidget;
        InventoryWidget inventoryWidget;
        ItemSlotWidget itemSlotWidget;
        ItemSlotWidget clearItemSlotWidget;
        TextWidget fpsText;

        FurnaceWidget furnaceWidget;

        Font font;

        PickaxeTool pickaxe;

        RigidbodyActor* physicsPrototype;

        RigidbodyActor* physicsActor;


        Material terrainMaterial = Material::none;
        Material terrainMaterialDebug = Material::none;


        Skybox skybox;

        std::atomic<bool> closing = false;
        std::atomic<bool> chunkLoadPaused = false;

        float frametimes[60];
        int currentFrameTimeIndex = 0;

        Mesh<Vertex> model;
        Material material = Material::none;
        

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
            input.currentMousePosition = vec2(xposIn,yposIn);
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

        void initWindow() {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            glfwSetErrorCallback(errorCallback);
            window = glfwCreateWindow(windowWidth, windowHeight, name.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

            //Inputs
            glfwSetKeyCallback(window,keyCallback);
            glfwSetCursorPosCallback(window, mouseCallback); 
            glfwSetCharCallback(window, characterCallback);
            glfwSetMouseButtonCallback(window, mouseButtonCallback);

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
            
            if (window == NULL) {
                std::cout << "Window didn't load properly" << std::endl;
                glfwTerminate();
                return;
            }
            
        }

        void setup() {

            // Load

            model.meshData.vertices.push_back(Vertex(vec3(0.0f,0.0f,0.0f)));
            model.meshData.indices.push_back(0);
            model.meshData.indices.push_back(1);
            model.meshData.indices.push_back(2);
            model.meshData.indices.push_back(3);
            model.meshData.indices.push_back(4);
            model.meshData.indices.push_back(5);
            model.createBuffers(vulkan);

            auto pipeline = vulkan->createManagedPipeline<Vertex>("shaders/compiled/test_vert.spv","shaders/compiled/test_frag.spv");

            material = vulkan->createMaterial(pipeline,LitMaterialData());

        }

        void loop() 
        {

            ZoneScoped;

            // Get inputs, window resize, and more
            glfwPollEvents();

            int frameWidth;
            int frameHeight;
            glfwGetFramebufferSize(window,&frameWidth,&frameHeight);

            // Get time
            float dt = (float)glfwGetTime() - lastTime;
            lastTime = glfwGetTime();

            camera.position = vec3(0,0,-10);

            if(input.getKey(GLFW_KEY_SPACE)) {
                camera.rotate(vec3(0,dt*30,0));
            }

            if(input.getKeyPressed(GLFW_KEY_P)) {
                vulkan->printVMAstats();
            }

            model.addToRender(vulkan,material,glm::mat4(1.0f));

            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            Debug::clearDebugShapes();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            FrameMark;
            

        }

        

        

        
};