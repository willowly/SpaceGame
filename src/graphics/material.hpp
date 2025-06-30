#pragma once
#include "shader.hpp"
#include "texture.hpp"
#include "color.hpp"

class Material {
    public:

        Material() {
            
        }
        Material(Shader* shader,Texture* texture,Color color = Color::white) : shader(shader), texture(texture), color(color) {
            
        }
        Shader* shader = nullptr;
        Texture* texture = nullptr;
        Color color = Color::white;


        void use() {
            if(shader != nullptr) {
                shader->use();
                shader->setColor3("color",color);
            }
            if(texture != nullptr) texture->bind();
        }
};