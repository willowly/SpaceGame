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

#include "engine/world.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"

#include "actor/actor.hpp"
#include "actor/character.hpp"
#include "actor/physics-actor.hpp"

#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>

#include "api/api-all.hpp"
#include "helper/collision-helper.hpp"

#include <sol/sol.hpp>

using std::vector,glm::vec3,glm::quat;

int main()
{

    // Set up context settings
    sf::ContextSettings settings;
    settings.attributeFlags = sf::ContextSettings::Core;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.antiAliasingLevel = 4;
    settings.majorVersion = 4;
    settings.minorVersion = 1;

    // set up window and opengl
    auto window = sf::RenderWindow(sf::VideoMode({1200, 800}), "Space Game",sf::State::Windowed,settings);
    window.setFramerateLimit(144);
    window.setMouseCursorVisible(false);

    std::cout << "OpenGL version " << window.getSettings().majorVersion << "." << window.getSettings().minorVersion << std::endl;

    glEnable(GL_DEPTH_TEST);

    // set up registry
    Registry registry;

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    API::loadAPIAll(lua);

    // load everything
    Loader loader;
    loader.loadAll(registry);
    
    lua["textures"] = API::TextureRegistry(registry);
    lua["shaders"] = API::ShaderRegistry(registry);
    lua["materials"] = API::MaterialRegistry(registry);
    lua.do_file("scripts/load.lua");

    Model quad;
    quad.loadQuad();

    //registry.textures.at("grid").setPointFiltering();
    
    World world;

    Material material(&registry.litShader,&registry.textures.at("cow"));
    
    Actor cubePrototype(&registry.models.at("cube"),&registry.materials.at("cow"));
    Actor containerPrototype(&registry.models.at("cube"),&registry.materials.at("container"));
    Actor planePrototype(&registry.models.at("plane"),&registry.materials.at("grid"));

    PhysicsActor physicsPrototype(&registry.models.at("cube"),&registry.materials.at("cow"));

    Character playerPrototype;
    playerPrototype.useGravity = true;
    
    

    world.spawn<Actor>(&planePrototype,vec3(0,-3,0),quat(1.0f,0.0f,0.0f,0.0f));
    auto physics = world.spawn(&physicsPrototype,vec3(1.0f,0.0f,0.0f),glm::identity<quat>());
    physics->useGravity = true;
    //Actor& cube = *world.spawn<Actor>(&cubePrototype,vec3(0,3,0),quat(1.0f,0.0f,0.0f,0.0f));
    //world.spawn<Actor>(&containerPrototype,vec3(0,5,0),quat(1.0f,0.0f,0.0f,0.0f));
    Character* playerActor = world.spawn(&playerPrototype,vec3(0,0,10),quat(1.0f,0.0f,0.0f,0.0f));
    

    Camera camera;
    camera.setAspect(window.getSize().x,window.getSize().y);


    sf::Vector2i screenCenter = sf::Vector2i((sf::Vector2f)window.getSize()/2.0f);

    sf::Mouse::setPosition((screenCenter + window.getPosition())*2);

    
    
    sf::Clock clock;
    float angle;
    glm::quat cubeRotation(1.0f,0.0f,0.0f,0.0f);
    vec3 cubePosition = vec3(0.0f);
    vec3 cubeHalfSize = vec3(1.0f);
    bool mousePressed = false;
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {

            // Events and Input
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

                    // Mouse looking
                    auto movement = sf::Mouse::getPosition(window) - screenCenter;
                    sf::Mouse::setPosition((screenCenter + window.getPosition())*2);

                    playerActor->moveMouse(vec2((float)movement.x,(float)movement.y) / 100.0f);
                }

            }
        }
        // the camera should look out from the 
        playerActor->firstPersonCamera(camera);

        // clear the buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float dt = clock.restart().asSeconds();
        
        
        world.render(camera);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(30*dt),vec3(0,1,0));
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(-30*dt),vec3(0,1,0));
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(30*dt),vec3(1,0,0));
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(-30*dt),vec3(1,0,0));
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Comma)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(30*dt),vec3(1,0,0));
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Period)) {
                cubeRotation = glm::rotate(cubeRotation,glm::radians(-30*dt),vec3(1,0,0));
            }
        } else {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt)) {
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                    cubeHalfSize += vec3(dt,0,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                    cubeHalfSize += vec3(-dt,0,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                    cubeHalfSize += vec3(0,0,dt);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                    cubeHalfSize += vec3(0,0,-dt);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Comma)) {
                    cubeHalfSize += vec3(0,dt,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Period)) {
                    cubeHalfSize += vec3(0,-dt,0);
                }
            } else {
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                    cubePosition += vec3(dt,0,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                    cubePosition += vec3(-dt,0,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                    cubePosition += vec3(0,0,dt);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                    cubePosition += vec3(0,0,-dt);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Comma)) {
                    cubePosition += vec3(0,dt,0);
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Period)) {
                    cubePosition += vec3(0,-dt,0);
                }
            }
        }

        
        
    
        world.step(dt);
        

        CollisionHelper::Ray ray(camera.position,camera.direction());
        

        Debug::renderDebugShapes(camera);

        auto hitOpt = physics->raycast(ray);

        if(hitOpt) {
            auto hit = hitOpt.value();
            Debug::drawPoint(hit.point);

            if(mousePressed) {
                if(!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    mousePressed = false;
                }
            } else {
                if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
                    mousePressed = true;
                    physics->applyForce(camera.direction() * 10.0f,hit.point);
                }
            }
        }

        physics->angularVelocity = physics->angularVelocity * pow(2.0f,-dt);
        
        vec3 ropePosition = vec3(0,5,0);
        Debug::drawLine(ropePosition,physics->position);
        vec3 ropeDelta = ropePosition - physics->position;
        float ropeForce = 10;
        float ropeLength = 4;
        if(glm::length(ropeDelta) > ropeLength) {
            physics->velocity += glm::dot(-glm::normalize(ropeDelta),physics->velocity) * glm::normalize(ropeDelta);
            ropeDelta = glm::normalize(ropeDelta) * ropeLength;
            physics->position = ropePosition - ropeDelta;
        }
        std::cout << StringHelper::toString(physics->position) << std::endl;


        window.display();
    }
}
