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
#include "actor/actor-factory.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"

using std::string;


class GameApplication {

    

    public:
        GameApplication(string name) : name(name) {
            
            initWindow();
            vulkan = new Vulkan(name,window);
                                                                
            pipeline = vulkan->createGraphicsPipeline<Vertex>("shaders/compiled/shader_vert.spv","shaders/compiled/shader_frag.spv");

        }

        ~GameApplication() {

            vkDestroyPipeline(vulkan->getDevice(),pipeline,nullptr);
            
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
        
        //Registry
        Model* model;
        TextureID cowTexture;
        TextureID gridTexture;
        Material cowMaterial;
        Material gridMaterial;

        Registry registry;

        Loader loader;
        
        sol::state lua;

        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        VkPipeline pipeline;

        Input input;

        std::vector<glm::mat4> monkeys; 

        vec3 rotation = vec3(0);
        
        Camera camera;

        std::vector<float> frameTimes;

        Character* player;

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

            model = new Model();
            model->loadFromFile("models/monkey.obj");

            lua.open_libraries(sol::lib::base, sol::lib::package);
            API::loadAPIAll(lua);

            loader.loadAll(registry,lua,vulkan);

            lastTime = (float)glfwGetTime();

            cowTexture = registry.getTexture("cow");

            gridTexture = registry.getTexture("grid_dark");

            cowMaterial = vulkan->createMaterial(pipeline,LitMaterialData(cowTexture,vec3(1)));

            gridMaterial = vulkan->createMaterial(pipeline,LitMaterialData(gridTexture,vec3(1)));

            Actor monkeyPrototype = Actor(model,gridMaterial);

            Character playerPrototype = Character();

            world.spawn<Actor>(&monkeyPrototype,vec3(0,0,0));

            player = world.spawn<Character>(&playerPrototype,vec3(0.0,0.0,5.0));


            // for (size_t i = 0; i < 5000; i++)
            // {
            //     auto position = vec3(Random::random(-3,3),Random::random(-3,3),Random::random(-3,3));
            //     auto quaternion = quat(vec3(Random::random(0,360),Random::random(0,360),Random::random(0,360)));
            //     auto scale = vec3(Random::random(0.01,0.1));
            //     monkeys.push_back(MathHelper::getTransformMatrix(position,quaternion,scale));
            // }

            
            

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

            if(input.getKey(GLFW_KEY_RIGHT)) {
                input.currentMousePosition.x += 10000;
                input.lastMousePosition.x += 10000;
            }
            
            world.frame(vulkan,dt);

            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            input.clearInputBuffers();

            
            

        }

        

        

        
};