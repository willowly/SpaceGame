#define GL_SILENCE_DEPRECATION
#include <SFML/Graphics.hpp>
#include <OpenGL/gl3.h>
#include <iostream>
#include "shader/shader.hpp"
#include "helper/file-helper.hpp"
#include "helper/color.hpp"
#include "include/stb_image.h"
#include "helper/model.hpp"
#include "helper/string-helper.hpp"

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>

using std::vector;

int main()
{

    sf::ContextSettings settings;
    settings.attributeFlags = sf::ContextSettings::Core;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.majorVersion = 4;
    settings.minorVersion = 1;

    auto window = sf::RenderWindow(sf::VideoMode({600, 600}), "Space Game",sf::State::Windowed,settings);
    window.setFramerateLimit(144);

    std::cout << "version " << window.getSettings().majorVersion << "." << window.getSettings().minorVersion << std::endl;

    glEnable(GL_DEPTH_TEST);

    Shader shader;
    shader.loadFromFiles("shaders/default.vert","shaders/default.frag");

    
    

    //create some vertices
    
    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

    

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);  
    glBindBuffer(GL_ARRAY_BUFFER, VBO);  
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // set up indicies
    // unsigned int EBO;
    // glGenBuffers(1, &EBO);
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indicesVector.size(), indicesVector.data(), GL_STATIC_DRAW);

    // // set up the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    // glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);


    

    // make the texture
    unsigned int texture;
    glGenTextures(1,&texture);

    // bind it
    glBindTexture(GL_TEXTURE_2D,texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    string path = "textures/cow.png";
    // load the image
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if(data) {

        // load the data to the gpu
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << data << std::endl;
    } else {
        std::cout << "[ERROR] could not load texture " + path << std::endl;
    }

    stbi_image_free(data); //get rid of the data

    // actually drawing!!

    Model model;
    model.loadFromFile("models/cube.obj");

    auto transform = glm::mat4(1.0f);

    glm::vec3 rotation;
    
    sf::Clock clock;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            } else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                // adjust the viewport when the window is resized
                glViewport(0, 0, resized->size.x, resized->size.y);
            }
        }
        float dt = clock.restart().asSeconds();

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindTexture(GL_TEXTURE_2D,texture);
        shader.use();
        glm::mat4 view = glm::mat4(1.0f);

        
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f)); 
        shader.setMat4("view",view);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        shader.setMat4("projection",projection);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
        model = glm::rotate(model, glm::radians(rotation.y),glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::rotate(model, glm::radians(rotation.x),glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("model",model);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            rotation += glm::vec3(0,dt * 90,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            rotation -= glm::vec3(0,dt * 90,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            rotation += glm::vec3(dt * 90,0,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            rotation -= glm::vec3(dt * 90,0,0);
        }

        glBindVertexArray(VAO);
        
        
        //shader.setColor("color",Color(sin(t*5)/2 + 0.5f,cos(t*5)/2 + 0.5f,0.2f));
        glDrawArrays(GL_TRIANGLES, 0, 36);


        window.display();
    }
}
