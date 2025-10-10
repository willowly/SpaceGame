#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
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

using std::string;


class GameApplication {

    

    public:
        GameApplication(string name) : name(name) {
            
            initWindow();
            vulkan = new Vulkan(name,window);

            model = new Model();

            pipeline = vulkan->createGraphicsPipeline<Vertex>();

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

        Model* model;


    private:
    
        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        VkPipeline pipeline;

        Input input;

        std::vector<glm::mat4> monkeys; 

        vec3 rotation = vec3(0);
        
        Camera camera;

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
            std::cout << "pointer = " << this << std::endl;
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

            model->loadFromFile("models/monkey.obj");
            model->createBuffers(vulkan);

            lastTime = (float)glfwGetTime();
            

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

            vec2 mouseDelta = -input.getMouseDelta();
            // test inputs
            camera.rotate(vec3(mouseDelta.y * dt,mouseDelta.x * dt,0));
            
            // set up vulkan to start rendering. It can return false if we need to try again (bc of window resize)
            if(!vulkan->startFrame(camera)) return;

            // test render
            vkCmdBindPipeline(vulkan->getCurrentCommandBuffer(),VK_PIPELINE_BIND_POINT_GRAPHICS,pipeline);


            auto position = vec3(Random::random(-3,3),Random::random(-3,3),Random::random(-3,3));
            auto quaternion = quat(vec3(Random::random(0,360),Random::random(0,360),Random::random(0,360)));
            auto scale = vec3(Random::random(0.01,0.1));
            monkeys.push_back(MathHelper::getTransformMatrix(position,quaternion,scale));

            for (auto mat : monkeys)
            {
                vulkan->drawMeshSingle(model->meshBuffer,mat);
            }
            
            

            // do all the end of frame code in vulkan
            vulkan->submitFrame();

            input.clearInputBuffers();

            
            

        }

        

        

        
};