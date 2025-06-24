#define GL_SILENCE_DEPRECATION

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include "graphics/shader.hpp"
#include "helper/file-helper.hpp"
#include "graphics/color.hpp"
#include "stb_image.h"
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

#include "graphics/text.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "api/api-all.hpp"
#include "helper/collision-helper.hpp"

#include <reactphysics3d/reactphysics3d.h>

#include <sol/sol.hpp>

using std::vector,glm::vec3,glm::quat;

int main()
{

    // Set up context settings
    

    // set up window and opengl
    


    glEnable(GL_DEPTH_TEST);

    // for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    reactphysics3d::PhysicsCommon physicsCommon;
    
    World world;
    world.setPhysicsCommon(&physicsCommon);
    auto physicsWorld = physicsCommon.createPhysicsWorld();
    rp3d::RigidBody* planeBody = physicsWorld->createRigidBody(rp3d::Transform(rp3d::Vector3(0,-3,0),rp3d::Quaternion::identity()));
    planeBody->addCollider(physicsCommon.createBoxShape(rp3d::Vector3(8.0f,1.0f,8.0f)),rp3d::Transform(rp3d::Vector3(0,-1,0),rp3d::Quaternion::identity()));
    planeBody->setType(rp3d::BodyType::STATIC);
    world.setPhysicsWorld(physicsWorld);
    

    Material material(&registry.litShader,&registry.textures.at("cow"));
    
    Actor cubePrototype(&registry.models.at("cube"),&registry.materials.at("cow"));
    Actor containerPrototype(&registry.models.at("cube"),&registry.materials.at("container"));
    Actor planePrototype(&registry.models.at("plane"),&registry.materials.at("grid"));

    PhysicsActor physicsPrototype(&registry.models.at("cube"),&registry.materials.at("cow"));
    

    Character playerPrototype;
    playerPrototype.useGravity = true;
    
    

    world.spawn<Actor>(&planePrototype,vec3(0,-3,0),quat(1.0f,0.0f,0.0f,0.0f));
    auto physics = world.spawn(&physicsPrototype,vec3(0.0f,0.0f,0.0f),quat(vec3(0.0f)));
    world.spawn(&physicsPrototype,vec3(0.0f,2.0f,1.0f),quat(vec3(0.0f)));
    world.spawn(&physicsPrototype,vec3(0.0f,4.0f,1.0f),quat(vec3(0.0f)));
    world.spawn(&physicsPrototype,vec3(0.0f,6.0f,1.0f),quat(vec3(0.0f)));

    Character* playerActor = world.spawn(&playerPrototype,vec3(0,0,10),quat(1.0f,0.0f,0.0f,0.0f));
    

    

    Camera camera;
    camera.setAspect(window.getSize().x,window.getSize().y);


    sf::Vector2i screenCenter = sf::Vector2i((sf::Vector2f)window.getSize()/2.0f);

    sf::Mouse::setPosition((screenCenter + window.getPosition())*2);


    Text text("fonts/courier-new.ttf",30);
    text.text = "hello world";
    text.position = glm::vec2(50,50);
    
    sf::Clock clock;
    float angle;
    glm::quat cubeRotation(1.0f,0.0f,0.0f,0.0f);
    vec3 cubePosition = vec3(0.0f);
    vec3 cubeHalfSize = vec3(1.0f);
    bool mousePressed = false;
    bool nextPressed = false;
    float sinceLastStep;
    float stepDt = 0.02f;
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
        sinceLastStep += dt;
        
        //world.render(camera);
        

        if(sinceLastStep > 0.1f) {
            sinceLastStep = 0.1f; //we dont want to get caught in an infinite loop! if the physics takes too long the game will just slow down
        }
        //max 5 steps before we render again (dont wanna get caught in an infinite loop!)
        while (sinceLastStep > stepDt)
        {
            sinceLastStep -= stepDt;
            world.step(stepDt);
            world.playerStep(stepDt);
        } 

        CollisionHelper::Ray ray(camera.position,camera.direction());

        
        
        glDisable(GL_DEPTH_TEST);
        //Debug::renderDebugShapes(camera);
        glEnable(GL_DEPTH_TEST);

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

        text.render(registry.textShader,glm::ortho(0.0f,100.0f,0.0f,100.0f));
        

        window.display();
    }
}
