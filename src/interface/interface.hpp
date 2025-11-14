#pragma once

#include "graphics/shader.hpp"
#include "graphics/model.hpp"
#include "graphics/color.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <helper/math-helper.hpp>

#include "engine/debug.hpp"


using glm::ivec2, glm::vec2;

class Interface {
    public:
        Shader* shader;
        Mesh quadModel;
        ivec2 screenSize;

        Interface(Shader* shader,ivec2 screenSize) : shader(shader), screenSize(screenSize) {
            quadModel.loadQuad();
        }
        


        void drawRect(vec2 position,vec2 size,vec2 anchor,Color color,Texture* texture) {
            glm::mat4 view = glm::mat4(1.0f);
            glm::mat4 model = glm::translate(glm::mat4(1.0f),vec3(position,1));
            model = glm::scale(model,vec3(size,1) * 0.5f); // quad is from -1 to 1, which is 2x too big
            glm::mat4 projection = glm::ortho(mh::lerp(-screenSize.x,0,anchor.x),mh::lerp(0,screenSize.x,anchor.x),mh::lerp(-screenSize.y,0,anchor.y),mh::lerp(0,screenSize.y,anchor.y));
            texture->bind();
            shader->use();
            shader->setColor("color",color);
            quadModel.render(view,model,projection,*shader);
        }





};