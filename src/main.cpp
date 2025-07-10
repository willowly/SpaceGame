#define GL_SILENCE_DEPRECATION

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>

#include <engine/loader.hpp>
#include <engine/world.hpp>
#include <engine/input.hpp>

#include <block/block.hpp>
#include <block/thruster-block.hpp>
#include <block/cockpit-block.hpp>

#include <interface/console.hpp>
#include <interface/interface.hpp>
#include <interface/toolbar-widget.hpp>

#include <actor/character.hpp>
#include <actor/terrain.hpp>
#include <actor/construction.hpp>

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
    Terrain terrainPrototype;
    terrainPrototype.material = registry.getMaterial("grid");
    terrainPrototype.generate();


    Text text("fonts/sburbits.ttf",16);
    text.scale = vec2(2,2);

    World world;
    Interface interface(&registry.uiShader,glm::ivec2(width,height));

    ToolbarWidget toolbarWidget;
    toolbarWidget.solidTexture = registry.getTexture("solid");

    Console console("fonts/sburbits.ttf");

    Model cursorModel;
    cursorModel.loadQuad();

    PlaceBlockTool placeTin(registry.getBlock("tin"));
    placeTin.icon = registry.getTexture("tin_plate");
    PlaceBlockTool placeCockpit(registry.getBlock("cockpit"));
    placeCockpit.icon = registry.getTexture("cockpit_item");
    PlaceBlockTool placeThruster(registry.getBlock("thruster"));
    placeThruster.icon = registry.getTexture("thruster_item");
    PickaxeTool pickaxe(registry.getModel("pickaxe"),registry.getMaterial("pickaxe"),vec3(0.2,-0.4,-0.5),quat(vec3(glm::radians(-5.0f),glm::radians(90.0f),glm::radians(-5.0f))));
    pickaxe.icon = registry.getTexture("pickaxe_item");

    playerPrototype.toolbar[0] = &placeTin;
    playerPrototype.toolbar[1] = &placeCockpit;
    playerPrototype.toolbar[2] = &placeThruster;
    playerPrototype.toolbar[3] = &pickaxe;

    auto player = world.spawn(&playerPrototype,vec3(0,1,10),quat(vec3(0,0,0)));

    auto construction = world.spawn(&constructionPrototype,vec3(0,2,0),quat(vec3(0,0,0)));
    construction->setBlock(ivec3(0,0,0),registry.getBlock("tin"),BlockFacing::FORWARD);
    lua["construction"] = construction;

    for (int i = 0; i < 50; i++)
    {
        int size = 150;
        float x = (random() % size) - size/2.0f;
        float y = (random() % size) - size/2.0f;
        float z = (random() % size) - size/2.0f;
        world.spawn(registry.getActor("asteroid"),vec3(x,y,z),glm::quat(vec3(0,0,0)));
    }
    
    
    int terrainConfig = 0;

    float lastTime = 0;

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

        if(input.getKeyPressed(GLFW_KEY_UP)) {
            terrainConfig++;
            //terrainPrototype.setConfig(terrainConfig);
        }
        if(input.getKeyPressed(GLFW_KEY_DOWN)) {
            terrainConfig--;
            //terrainPrototype.setConfig(terrainConfig);
        }
        
        terrainPrototype.setConfig(terrainConfig);
        terrainPrototype.render(world.getCamera(),dt);

        world.frame(dt);

        auto matrix = glm::ortho(0.0f,(float)width,0.0f,(float)height);
        text.text = std::format("FPS: {}",(int)(1.0f/dt));
        text.render(registry.textShader,matrix);

        if(console.enabled) console.render(vec2(width,height),registry.textShader);

        glDisable(GL_DEPTH_TEST);
        Debug::renderDebugShapes(world.getCamera());

        interface.screenSize = glm::ivec2(width,height);
        toolbarWidget.player = player;
        toolbarWidget.render(interface);

        cursorModel.render(glm::mat4(1.0f),glm::scale(glm::mat4(1.0f),vec3(4,4,4)),glm::ortho(-width/2.0f,width/2.0f,-height/2.0f,height/2.0f),*Debug::getShader());
        glEnable(GL_DEPTH_TEST);

        //registry.getModel("cockpit")->render(vec3(0,3,0),world.getCamera(),*registry.getMaterial("cockpit"));
        
        glfwSwapBuffers(window);
        input.clearInputBuffers(); //do it before we poll for events
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}
