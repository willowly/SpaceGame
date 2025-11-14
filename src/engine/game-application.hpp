#pragma once
#define VULKAN_NO_PROTOTYPES
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <format>
#include <set>
#include "helper/file-helper.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/model.hpp"
#include "engine/input.hpp"
#include "helper/string-helper.hpp"
#include "helper/random-helper.hpp"
#include "graphics/camera.hpp"
#include "world.hpp"
#include "actor/character.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "helper/clock.hpp"

#include "item/place-block-tool.hpp"

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
        uint32_t windowWidth = 800;
        uint32_t windowHeight = 600;
        void run() {

            setup();

            while (!glfwWindowShouldClose(window)) {
                loop();
            }

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


        PlaceBlockTool placeTin;
        PlaceBlockTool placeCockpit;
        PlaceBlockTool placeThruster;
        PickaxeTool pickaxe;

        MeshBuffer uiBuffer;
        VkPipeline uiPipeline;
        Material uiMaterial = Material::none;


        

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
            lua.open_libraries(sol::lib::base, sol::lib::package);
            API::loadAPIAll(lua);
            loader.loadAll(registry,lua,vulkan);


            lastTime = (float)glfwGetTime();

            Actor* planePrototype = registry.getActor("plane");

            auto playerPrototype = Character::makeDefaultPrototype();

            world.spawn(Actor::makeInstance(planePrototype,vec3(0,-3,0)));

            placeTin = PlaceBlockTool(registry.getBlock("tin"));
            placeTin.icon = registry.getTexture("tin_plate");

            placeCockpit = PlaceBlockTool(registry.getBlock("cockpit"));
            placeCockpit.icon = registry.getTexture("cockpit_item");

            placeThruster = PlaceBlockTool(registry.getBlock("thruster"));
            placeThruster.icon = registry.getTexture("thruster_item");

            pickaxe = PickaxeTool(registry.getModel("pickaxe"),registry.getMaterial("pickaxe"),vec3(0.2,-0.4,-0.5),quat(vec3(glm::radians(-5.0f),glm::radians(90.0f),glm::radians(-5.0f))));
            pickaxe.icon = registry.getTexture("pickaxe_item");

            //world.spawn(Construction::makeInstance(tin,vec3(0)));

            playerPrototype->toolbar[0] = &placeTin;
            playerPrototype->toolbar[1] = &placeCockpit;
            playerPrototype->toolbar[2] = &placeThruster;
            playerPrototype->toolbar[3] = &pickaxe;

            std::vector<Vertex> verticies{
                Vertex(vec3(0.0,0.0,0.0))
            };
            std::vector<uint16_t> indicies{
                0,1,2,1,2,3
            };
            uiBuffer = vulkan->createMeshBuffers(verticies,indicies);

            player = world.spawn(Character::makeInstance(playerPrototype.get(),vec3(0.0,0.0,0.0)));

            uiPipeline = vulkan->createManagedPipeline<Vertex>("shaders/compiled/ui_vert.spv","shaders/compiled/ui_frag.spv");

            uiMaterial = vulkan->createMaterial(uiPipeline,LitMaterialData());

            glfwPollEvents();
            input.clearInputBuffers(); // reset mouse position;
            
            

        }

        void loop() {

            // Get inputs, window resize, and more
            glfwPollEvents();

            int frameWidth;
            int frameHeight;
            glfwGetFramebufferSize(window,&frameWidth,&frameHeight);

            // Get time
            float dt = (float)glfwGetTime() - lastTime;
            lastTime = glfwGetTime();

            camera.setAspect(frameWidth,frameHeight);
            
            // test inputs
            //camera.rotate(vec3(mouseDelta.y * dt,mouseDelta.x * dt,0));
            camera.rotate(vec3(0,dt*-10,0));

            frameTimes.push_back(dt);

            if(frameTimes.size() >= 10) {
                float average = 0;
                for (size_t i = 0; i < frameTimes.size(); i++)
                {
                    average += frameTimes[i];
                }

                average /= frameTimes.size();
                std::cout << "fps: " << (1/average) << std::endl;
                
                frameTimes.clear();
            }

            player->setCamera(camera);
            player->processInput(input);
            
            world.frame(vulkan,dt);

            vulkan->addMesh(uiBuffer,uiMaterial,glm::mat4(1.0f));
            
            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            
            

        }

        

        

        
};