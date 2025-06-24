#pragma once
#include <iostream>
#include <fstream>
#include "helper/file-helper.hpp"
#include "graphics/color.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::string, std::ofstream;

class Shader {
    
    
    public:

        Shader() {
            shaderProgram = glCreateProgram();
        }
        ~Shader() {
            glDeleteProgram(shaderProgram);
        }

        unsigned int shaderProgram;

        void setColor(string paramName,Color color) {
            int paramLocation = glGetUniformLocation(shaderProgram, paramName.c_str());
            glUniform4f(paramLocation, color.r, color.g, color.b, color.a);
        }

        void setColor3(string paramName,Color color) {
            int paramLocation = glGetUniformLocation(shaderProgram, paramName.c_str());
            glUniform3f(paramLocation, color.r, color.g, color.b);
        }

        void setMat4(string paramName,glm::mat4 matrix) {
            int paramLocation = glGetUniformLocation(shaderProgram, paramName.c_str());
            glUniformMatrix4fv(paramLocation, 1, GL_FALSE,glm::value_ptr(matrix));
        }

        void setViewModelProjection(glm::mat4 view,glm::mat4 model,glm::mat4 projection) {
            setMat4("view",view);
            setMat4("model",model);
            setMat4("projection",projection);
        }

        void loadFromFiles(string vertexShaderPath,string fragmentShaderPath) {
            string vertexShaderSource = FileHelper::readToString(vertexShaderPath);
            string fragmentShaderSource = FileHelper::readToString(fragmentShaderPath);
            loadFromMemory(vertexShaderSource.c_str(),fragmentShaderSource.c_str());
        }
    
        void loadFromMemory(const char* vertexShaderSource,const char* fragmentShaderSource) {

            // create the shaders and assign them
            unsigned int vertexShader;
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
            glCompileShader(vertexShader);

            unsigned int fragmentShader;
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
            glCompileShader(fragmentShader);
            
            // check success
            int  success;
            char infoLog[512];
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                std::cout << "[ERROR] Vertex shader compilation error:\n" << infoLog << std::endl;
            }
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if(!success)
            {
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                std::cout << "[ERROR] Fragment shader compilation error:\n" << infoLog << std::endl;
            }

            //now we need to make a shader program to make it all work
            glAttachShader(shaderProgram, vertexShader);
            glAttachShader(shaderProgram, fragmentShader);
            glLinkProgram(shaderProgram);

            // get the success of the linking process
            glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
            if(!success) {
                glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
                std::cout << "[ERROR] Shader linking error:\n" << infoLog << std::endl;
            }
            // we gotta delete the shaders after we've put them in the program
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
        }

        void use() {
            glUseProgram(shaderProgram);
        }
};