#pragma once
#include "glfw/glfw3.h"


class Clock {
    double time;

    public:
        Clock() {
            time = glfwGetTime();
        }
        float getTime() {
            return glfwGetTime() - time;
        }
        float reset() {
            float lastTime = glfwGetTime() - time;
            time = glfwGetTime();
            return lastTime;
        }
};