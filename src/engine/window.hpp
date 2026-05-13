#pragma once
#include <GLFW/glfw3.h> // this could cause issues...

#include "input.hpp"

#include <string>
#include <iostream>
#include <format>

#include "glm/glm.hpp"

using std::string;
using glm::ivec2;

enum class CursorMode {
    Normal,
    Locked
};

class Window {

    GLFWwindow* glfwWindow = nullptr;

    Input input;

    bool framebufferResized;

    public:
        static void errorCallback(int error, const char* description) {
            std::cout << std::format("GLFW Error: {}\n {}",error,description) << std::endl;
        }

        static void framebufferResizeCallback(GLFWwindow* glfwWindow, int width, int height) {
            auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            window->framebufferResized = true;
            //window->vulkan->setFrameBufferResized();
        }

        static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
        {
            auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            auto& input = window->input;
            if(action == GLFW_PRESS) {
                input.keys[key] = true;
                input.keysPressed[key] = true;
            }
            if(action == GLFW_RELEASE) {
                input.keys[key] = false;
                input.keysReleased[key] = true;
            }
        }

        static void characterCallback(GLFWwindow* glfwWindow, unsigned int codepoint)
        {
            auto window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
            auto& input = window->input;
            input.textInput += (char)codepoint;
        }



        static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
        {
            auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            #if __APPLE__
                input.currentMousePosition = vec2(xposIn,yposIn)* 2.0f; //retina bs. Doesn't actually detect retina so itll probably not work on macs that have no retina
            #else
                input.currentMousePosition = vec2(xposIn,yposIn);
            #endif
        }

        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
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
        Window(string name,uint32_t windowWidth,uint32_t windowHeight) {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            glfwSetErrorCallback(errorCallback);
            glfwWindow = glfwCreateWindow(windowWidth, windowHeight, name.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(glfwWindow, this);
            glfwSetFramebufferSizeCallback(glfwWindow, framebufferResizeCallback);

            //Inputs
            glfwSetKeyCallback(glfwWindow,keyCallback);
            glfwSetCursorPosCallback(glfwWindow, mouseCallback); 
            glfwSetCharCallback(glfwWindow, characterCallback);
            glfwSetMouseButtonCallback(glfwWindow, mouseButtonCallback);
            
            
            if (glfwWindow == NULL) {
                std::cout << "Window didn't load properly" << std::endl;
                glfwTerminate();
                return;
            }
        }

        bool shouldClose() {
            assert(glfwWindow != nullptr);
            return glfwWindowShouldClose(glfwWindow);
        }

        ivec2 getFrameBufferSize() {
            int width, height;
            glfwGetFramebufferSize(glfwWindow, &width, &height);
            return ivec2(width,height);
        }

        bool getFrameBufferResized() {
            bool r = framebufferResized;
            framebufferResized = false;
            return r;
        }

        void setCursorMode(CursorMode mode) {
            assert(glfwWindow != nullptr);
            switch(mode) {
                case CursorMode::Normal:
                    glfwSetInputMode(glfwWindow,GLFW_CURSOR,GLFW_CURSOR_NORMAL);
                    break;
                case CursorMode::Locked:
                    glfwSetInputMode(glfwWindow,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
                    break;
            }
        }

        // required to update closed state
        const Input pollInput() {
            glfwPollEvents();
            Input returnedInput = input;
            input.clearInputBuffers();
            return returnedInput;
        }

        #ifdef RENDERER_VULKAN
            void createSurface(VkInstance instance, VkSurfaceKHR* surface) {
                assert(glfwWindow != nullptr);

                if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, surface) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create window surface!");
                }

            }

            void initImguiForVulkan() {
                ImGui_ImplGlfw_InitForVulkan(glfwWindow,true);
            }
        #endif

        ~Window() {

            glfwDestroyWindow(glfwWindow);

            glfwTerminate();
        }

};