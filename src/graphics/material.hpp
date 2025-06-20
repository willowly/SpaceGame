#pragma once
#include "shader.hpp"
#include "texture.hpp"

class Material {
    public:

        Material() {
            
        }
        Material(Shader* shader,Texture* texture) : shader(shader), texture(texture) {

        }
        Shader* shader = nullptr;
        Texture* texture = nullptr;

        void use() {
            if(texture != nullptr) shader->use();
            if(texture != nullptr) texture->bind();
        }
};