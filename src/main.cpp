#define GL_SILENCE_DEPRECATION
#include <SFML/Graphics.hpp>
#include <OpenGL/gl3.h>
#include <iostream>
#include "graphics/shader.hpp"
#include "helper/file-helper.hpp"
#include "graphics/color.hpp"
#include "include/stb_image.h"
#include "graphics/model.hpp"
#include "helper/string-helper.hpp"
#include "graphics/texture.hpp"
#include "graphics/camera.hpp"

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
    window.setMouseCursorVisible(false);
    //window.setMouseCursorGrabbed(true);

    

    std::cout << "version " << window.getSettings().majorVersion << "." << window.getSettings().minorVersion << std::endl;

    glEnable(GL_DEPTH_TEST);

    Shader shader;
    shader.loadFromFiles("shaders/default.vert","shaders/default.frag");

    Model model;
    model.loadFromFile("models/cube.obj",Model::DataFormat::PositionUV);
    model.shader = &shader;

    Texture texture;
    texture.loadFromFile("textures/cow.png");

    Camera camera;
    //camera.move(vec3(0,0,8.0f));
    camera.setAspect(window.getSize().x,window.getSize().y);

    // actually drawing!!

    sf::Vector2i screenCenter = sf::Vector2i((sf::Vector2f)window.getSize()/2.0f);

    sf::Mouse::setPosition((screenCenter + window.getPosition())*2);
    
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
                camera.setAspect(resized->size.x,resized->size.y);
                screenCenter = sf::Vector2i((sf::Vector2f)window.getSize()/2.0f);
            } else if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if(window.hasFocus()) {
                    auto movement = sf::Mouse::getPosition(window) - screenCenter;
                    sf::Mouse::setPosition((screenCenter + window.getPosition())*2);

                    camera.rotate(vec2(movement.x,movement.y) * 0.1f);
                }

            }
        }
        float dt = clock.restart().asSeconds();

        float moveSpeed = 5.0f;
        vec3 moveVector = vec3(0);
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            moveVector += vec3(0,0,-1);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            moveVector += vec3(0,0,1);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
            moveVector += vec3(-1,0,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
            moveVector += vec3(1,0,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
            moveVector += vec3(0,1,0);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            moveVector += vec3(0,-1,0);
        }

        camera.move(moveVector * glm::angleAxis(glm::radians(camera.yaw),vec3(0,1,0)) * dt * moveSpeed);

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        texture.bind();
        shader.use();


        
        model.render(camera.getViewMatrix(),glm::mat4(1.0f),camera.getProjectionMatrix());


        window.display();
    }
}
