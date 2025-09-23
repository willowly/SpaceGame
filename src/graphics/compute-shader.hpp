#pragma once
#include <iostream>
#include <fstream>
#include "helper/file-helper.hpp"
#include "graphics/color.hpp"
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string, std::ofstream;

class ComputeShader {

    public:
        unsigned int shaderProgram;

        ComputeShader() {
            shaderProgram = glCreateProgram();
        }
        ~ComputeShader() {
            glDeleteProgram(shaderProgram);
        }


        void loadFromFile(string path) {
            string source = FileHelper::readToString(path);
            loadFromMemory(source.c_str());
        }
    
        void loadFromMemory(const char* source) {

            unsigned int computeShader;
            computeShader = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(computeShader, 1, &source, NULL);
            glCompileShader(computeShader);
        }

};