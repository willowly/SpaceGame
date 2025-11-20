#pragma once
#define VULKAN_NO_PROTOTYPES
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <format>
#include <set>
#include "helper/file-helper.hpp"
#include "graphics/vulkan.hpp"
#include "graphics/mesh.hpp"
#include "engine/input.hpp"
#include "helper/string-helper.hpp"
#include "helper/random-helper.hpp"
#include "graphics/camera.hpp"
#include "world.hpp"
#include "actor/actors-all.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "interface/interface.hpp"
#include "helper/clock.hpp"


#include "interface/toolbar-widget.hpp"
#include "interface/inventory-widget.hpp"

#include "item/items-all.hpp"

using std::string;


class GameApplication {

    

    public:
        GameApplication(string name) : name(name) {
            
            initWindow();
            vulkan = new Vulkan(name,window);

        }

        ~GameApplication() {
            
            delete vulkan;

            glfwDestroyWindow(window);

            glfwTerminate();
        }
        string name;
        uint32_t windowWidth = 800;
        uint32_t windowHeight = 600;
        void run() {

            setup();

            while (!glfwWindowShouldClose(window)) {
                loop();
            }

            vulkan->waitIdle();

        }

        
        
        private:

        World world;
        

        Registry registry;

        Loader loader;
        
        sol::state lua;

        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        Input input;
        
        Camera camera;

        std::vector<float> frameTimes;

        Character* player;

        Interface interface;

        ToolbarWidget toolbarWidget;
        InventoryWidget inventoryWidget;

        Font font;


        Material terrainMaterial = Material::none;


        PlaceBlockTool placeTin;
        PlaceBlockTool placeCockpit;
        PlaceBlockTool placeThruster;
        PickaxeTool pickaxe;


        ResourceItem stoneItem;
        ResourceItem copperItem;
        ResourceItem rubyItem;


        

        float lastTime = 0; //tells how long its been since the last update

        static void errorCallback(int error, const char* description) {
            std::cout << std::format("GLFW Error: {}\n {}",error,description) << std::endl;
        }

        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            app->vulkan->setFrameBufferResized();
        }

        static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            if(action == GLFW_PRESS) {
                input.keys[key] = true;
                input.keysPressed[key] = true;
            }
            if(action == GLFW_RELEASE) {
                input.keys[key] = false;
                input.keysReleased[key] = true;
            }
        }

        static void characterCallback(GLFWwindow* window, unsigned int codepoint)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            input.textInput += (char)codepoint;
        }



        static void mouseCallback(GLFWwindow* window, double xposIn, double yposIn)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            input.currentMousePosition = vec2(xposIn,yposIn);
        }

        static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
        {
            auto app = reinterpret_cast<GameApplication*>(glfwGetWindowUserPointer(window));
            auto& input = app->input;
            if(action == GLFW_PRESS) {
                input.mouseButtons[button] = true;
                input.mouseButtonsPressed[button] = true;
            }
            if(action == GLFW_RELEASE) {
                input.mouseButtons[button] = false;
                input.mouseButtonsReleased[button] = true;
            }
        }

        void initWindow() {
            glfwInit();

            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            glfwSetErrorCallback(errorCallback);
            window = glfwCreateWindow(windowWidth, windowHeight, name.c_str(), nullptr, nullptr);
            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

            //Inputs
            glfwSetKeyCallback(window,keyCallback);
            glfwSetCursorPosCallback(window, mouseCallback); 
            glfwSetCharCallback(window, characterCallback);
            glfwSetMouseButtonCallback(window, mouseButtonCallback);

            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            
            
            if (window == NULL) {
                std::cout << "Window didn't load properly" << std::endl;
                glfwTerminate();
                return;
            }
            
        }

        void setup() {

            // Load
            lua.open_libraries(sol::lib::base, sol::lib::package);
            API::loadAPIAll(lua);
            loader.loadAll(registry,lua,vulkan);


            lastTime = (float)glfwGetTime();

            Actor* planePrototype = registry.getActor("plane");

            auto playerPrototype = Character::makeDefaultPrototype();

            world.spawn(Actor::makeInstance(planePrototype,vec3(0,-3,0)));

            placeTin = PlaceBlockTool(registry.getBlock("tin"));
            placeTin.icon = registry.getTexture("tin_plate");

            placeCockpit = PlaceBlockTool(registry.getBlock("cockpit"));
            placeCockpit.icon = registry.getTexture("cockpit_item");

            placeThruster = PlaceBlockTool(registry.getBlock("thruster"));
            placeThruster.icon = registry.getTexture("thruster_item");

            pickaxe = PickaxeTool(registry.getModel("pickaxe"),registry.getMaterial("pickaxe"),vec3(0.2,-0.4,-0.5),quat(vec3(glm::radians(-5.0f),glm::radians(90.0f),glm::radians(-5.0f))));
            pickaxe.icon = registry.getTexture("pickaxe_item");

            //world.spawn(Construction::makeInstance(tin,vec3(0)));

            VkPipeline terrainPipeline = vulkan->createManagedPipeline<TerrainVertex>(Vulkan::vertCodePath("terrain"),Vulkan::fragCodePath("terrain"));
            terrainMaterial = vulkan->createMaterial(terrainPipeline,LitMaterialData(registry.getTexture("rock")));
            
            Terrain* terrain = world.spawn(Terrain::makeInstance(terrainMaterial,vec3(0,0,10)));
            terrain->rockTexture = registry.getTexture("rock");
            terrain->oreTexture = registry.getTexture("ore");
            terrain->generateMesh(); // needs to generate after the texture is applied. information for this should be passed into the terrain material

            stoneItem.icon = registry.getSprite("stone_item");
            stoneItem.name = "Stone";
            terrain->item1 = &stoneItem;

            copperItem.icon = registry.getSprite("copper_item");
            copperItem.name = "Copper";
            terrain->item2 = &copperItem;

            rubyItem.icon = registry.getSprite("ruby_item");
            rubyItem.name = "Ruby";
            terrain->item3 = &rubyItem;

            playerPrototype->toolbar[0] = &placeTin;
            playerPrototype->toolbar[1] = &placeCockpit;
            playerPrototype->toolbar[2] = &placeThruster;
            playerPrototype->toolbar[3] = &pickaxe;

            player = world.spawn(Character::makeInstance(playerPrototype.get(),vec3(0.0,0.0,0.0)));

            glfwPollEvents();
            input.clearInputBuffers(); // reset mouse position;

            Debug::loadRenderResources(*vulkan);
            interface.loadRenderResources(*vulkan);

            toolbarWidget.player = player;
            toolbarWidget.solidSprite = registry.getSprite("solid");

            inventoryWidget.player = player;
            inventoryWidget.solid = registry.getSprite("solid");
            inventoryWidget.font = &font;

            font.texture = registry.getTexture("numbers");
            font.start = '0';
            font.charSize = vec2(8,12);
            font.textureSize = vec2(88,12);
            

        }

        void loop() {

            // Get inputs, window resize, and more
            glfwPollEvents();

            int frameWidth;
            int frameHeight;
            glfwGetFramebufferSize(window,&frameWidth,&frameHeight);

            // Get time
            float dt = (float)glfwGetTime() - lastTime;
            lastTime = glfwGetTime();

            camera.setAspect(frameWidth,frameHeight);
            
            // test inputs
            //camera.rotate(vec3(mouseDelta.y * dt,mouseDelta.x * dt,0));
            camera.rotate(vec3(0,dt*-10,0));

            player->setCamera(camera);
            player->processInput(input);

            world.raycast(player->getLookRay(),10);
            
            world.frame(vulkan,dt);

            // cursor
            interface.drawRect(*vulkan,Rect::centered(vec2(4,0.5)),vec2(0.5,0.5),Color::white,0);
            interface.drawRect(*vulkan,Rect::centered(vec2(0.5,4)),vec2(0.5,0.5),Color::white,0);


            toolbarWidget.draw(interface,*vulkan);
            inventoryWidget.draw(interface,*vulkan);

            // auto hitOpt = world.raycast(player->getLookRay(),10);
            // if(hitOpt) {
            //     auto hit = hitOpt.value();
                
            //     Debug::drawRay(hit.hit.point,hit.hit.normal,Color::green);
            // }

            //Debug::addRenderables(*vulkan);
            
            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            Debug::clearDebugShapes();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            
            

        }

        

        

        
};