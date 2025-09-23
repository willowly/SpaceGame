#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <format>
#include <set>
#include "helper/file-helper.hpp"
#include "vulkan.hpp"

using std::string;


class GameApplication {

    

    public:
        GameApplication(string name) : name(name) {}
        string name;
        uint32_t windowWidth = 800;
        uint32_t windowHeight = 600;
        void run() {
            initWindow();
            initVulkan();
            mainLoop();
            cleanUp();
        }


    private:
    
        GLFWwindow* window = nullptr;   
        
        Vulkan vulkan;

        static void errorCallback(int error, const char* description) {
            std::cout << std::format("GLFW Error: {}\n {}",error,description) << std::endl;
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            app->vulkan.setFrameBufferResized();
        }

        void initWindow() {

            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            glfwSetErrorCallback(errorCallback);
            window = glfwCreateWindow(windowWidth, windowHeight, name.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
            
            
            if (window == NULL) {
                std::cout << "Window didn't load properly" << std::endl;
                glfwTerminate();
                return;
            }
        }

        void initVulkan() {
            vulkan.init(name,window);
        }


        void mainLoop() {

            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                vulkan.drawFrame();
            }

            
            vulkan.waitIdle();

        }

        void cleanUp() {

            //clean up vulkan
            vulkan.cleanUp();

            glfwDestroyWindow(window);

            glfwTerminate();

        }

        

        

        
};