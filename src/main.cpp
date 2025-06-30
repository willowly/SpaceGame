#define GL_SILENCE_DEPRECATION

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>

#include <engine/loader.hpp>
#include <engine/world.hpp>
#include <engine/input.hpp>
#include <actor/construction.hpp>

#include <block/block.hpp>

#include <interface/console.hpp>

#include <actor/character.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <graphics/text.hpp>

#include "api/api-all.hpp"

using glm::vec3;

Input input;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "resize " << width << " " << height << std::endl;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action == GLFW_PRESS) {
        input.keys[key] = true;
        input.keysPressed[key] = true;
    }
    if(action == GLFW_RELEASE) {
        input.keys[key] = false;
        input.keysReleased[key] = true;
    }
}

void character_callback(GLFWwindow* window, unsigned int codepoint)
{
    input.textInput += (char)codepoint;
}



void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    input.currentMousePosition = vec2(xposIn,yposIn);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(action == GLFW_PRESS) {
        input.mouseButtons[button] = true;
        input.mouseButtonsPressed[button] = true;
    }
    if(action == GLFW_RELEASE) {
        input.mouseButtons[button] = false;
        input.mouseButtonsReleased[button] = true;
    }
}


int main()
{

    // ---------------- SET UP GLFW WINDOW ------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "SpaceGame", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int width;
    int height;
    glfwGetFramebufferSize(window,&width,&height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    glEnable(GL_DEPTH_TEST);
    // for text
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glfwSetKeyCallback(window,key_callback);
    glfwSetCursorPosCallback(window, mouse_callback); 
    glfwSetCharCallback(window, character_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    // ---------------- SET UP ENGINE ------------------
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package);
    API::loadAPIAll(lua);
    
    Registry registry;
    Loader loader;
    loader.loadAll(registry,lua);
    
    RigidbodyActor cubePrototype(registry.getModel("cube"),registry.getMaterial("cow"));
    Character playerPrototype(nullptr,nullptr);

    Construction constructionPrototype(registry.getModel("block"),registry.getMaterial("cow"));

    Block tinBlock(registry.getModel("block"),registry.getMaterial("tin_block"));
    Block cobaltBlock(registry.getModel("block"),registry.getMaterial("cobalt_block"));
    Block chairBlock(registry.getModel("chair"),registry.getMaterial("chair"));
    chairBlock.canRide = true;
    Block thrusterBlock(registry.getModel("thruster"),registry.getMaterial("thruster"));


    Text text("fonts/sburbits.ttf",16);
    text.scale = vec2(2,2);

    World world;

    Console console("fonts/sburbits.ttf");

    Model cursorModel;
    cursorModel.loadQuad();
    
    
    playerPrototype.placeTin.heldModel = registry.getModel("block");
    playerPrototype.placeTin.heldModelMaterial = registry.getMaterial("tin_block");
    playerPrototype.placeTin.modelOffset = vec3(0.3,-0.3,-1);
    playerPrototype.placeTin.modelRotation = quat(vec3(0,glm::radians(45.0f),0));
    playerPrototype.placeTin.modelScale = 0.3f;
    playerPrototype.placeTin.lookLerp = 10;
    playerPrototype.placeTin.block = &tinBlock;

    playerPrototype.placeCobalt.heldModel = registry.getModel("block");
    playerPrototype.placeCobalt.heldModelMaterial = registry.getMaterial("cobalt_block");
    playerPrototype.placeCobalt.modelOffset = vec3(0.3,-0.3,-1);
    playerPrototype.placeCobalt.modelRotation = quat(vec3(0,glm::radians(45.0f),0));
    playerPrototype.placeCobalt.modelScale = 0.3f;
    playerPrototype.placeCobalt.lookLerp = 10;
    playerPrototype.placeCobalt.block = &cobaltBlock;

    playerPrototype.placeChair.heldModel = registry.getModel("chair");
    playerPrototype.placeChair.heldModelMaterial = registry.getMaterial("chair");
    playerPrototype.placeChair.modelOffset = vec3(0.3,-0.3,-1);
    playerPrototype.placeChair.modelRotation = quat(vec3(0,glm::radians(45.0f),0));
    playerPrototype.placeChair.modelScale = 0.3f;
    playerPrototype.placeChair.lookLerp = 10;
    playerPrototype.placeChair.block = &chairBlock;

    playerPrototype.placeThruster.heldModel = registry.getModel("thruster");
    playerPrototype.placeThruster.heldModelMaterial = registry.getMaterial("thruster");
    playerPrototype.placeThruster.modelOffset = vec3(0.3,-0.3,-1);
    playerPrototype.placeThruster.modelRotation = quat(vec3(0,glm::radians(45.0f),0));
    playerPrototype.placeThruster.modelScale = 0.3f;
    playerPrototype.placeThruster.lookLerp = 10;
    playerPrototype.placeThruster.block = &thrusterBlock;

    playerPrototype.pickaxe.heldModel = registry.getModel("pickaxe");
    playerPrototype.pickaxe.heldModelMaterial = registry.getMaterial("pickaxe");
    playerPrototype.pickaxe.modelOffset = vec3(0.2,-0.4,-0.5);
    playerPrototype.pickaxe.modelRotation = quat(vec3(glm::radians(-5.0f),glm::radians(90.0f),glm::radians(-5.0f)));
    playerPrototype.pickaxe.modelScale = 0.3f;
    playerPrototype.pickaxe.lookLerp = 8;

    

    //world.getCamera().move(vec3(0,0,5));


    auto player = world.spawn(&playerPrototype,vec3(0,1,10),quat(vec3(0,0,0)));

    auto construction = world.spawn(&constructionPrototype,vec3(0,2,0),quat(vec3(0,0,0)));
    construction->setBlock(ivec3(0,0,0),&tinBlock);
    lua["construction"] = construction;

    lua["rot"] = [&] (float x,float y,float z) {
        player->pickaxe.modelRotation = quat(vec3(glm::radians(x),glm::radians(y),glm::radians(z)));
    };

    for (int i = 0; i < 50; i++)
    {
        int size = 150;
        float x = (random() % size) - size/2.0f;
        float y = (random() % size) - size/2.0f;
        float z = (random() % size) - size/2.0f;
        world.spawn(registry.getActor("asteroid"),vec3(x,y,z),glm::quat(vec3(0,0,0)));
    }
    
    


    float lastTime;

    while(!glfwWindowShouldClose(window))
    {

        glClearColor(0.0f, 0.0f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetWindowSize(window,&width,&height);
        world.getCamera().setAspect(width,height);
        
        float dt = (float)glfwGetTime() - lastTime;
        lastTime = glfwGetTime();

        player->setCamera(world.getCamera());
        
        if(input.getKeyPressed(GLFW_KEY_GRAVE_ACCENT)) {
            console.enabled = !console.enabled;
        }
        if(console.enabled) {
            console.processInput(input,lua);
        } else {
            player->processInput(input);
        }

        world.frame(dt);

        auto matrix = glm::ortho(0.0f,(float)width,0.0f,(float)height);
        text.text = std::format("FPS: {}",(int)(1.0f/dt));
        text.render(registry.textShader,matrix);

        if(console.enabled) console.render(vec2(width,height),registry.textShader);

        glDisable(GL_DEPTH_TEST);
        Debug::renderDebugShapes(world.getCamera());

        cursorModel.render(glm::mat4(1.0f),glm::scale(glm::mat4(1.0f),vec3(4,4,4)),glm::ortho(-width/2.0f,width/2.0f,-height/2.0f,height/2.0f),*Debug::getShader());
        glEnable(GL_DEPTH_TEST);
        
        glfwSwapBuffers(window);
        input.clearInputBuffers(); //do it before we poll for events
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}
