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
#include "graphics/skybox.hpp"
#include "world.hpp"
#include "actor/actors-all.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "interface/interface.hpp"
#include "helper/clock.hpp"
#include <thread>
#include <chrono>

#include <tracy/Tracy.hpp>
#include <tracy/TracyVulkan.hpp>

#include "interface/actor/player-widget.hpp"
#include "interface/block/furnace-widget.hpp"

#include "item/items-all.hpp"
#include "item/recipe.hpp"

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

            std::cout << "main thread id:" << std::this_thread::get_id() << std::endl;

            setup();

            std::thread thread(&GameApplication::chunkTask,this);
            

            while (!glfwWindowShouldClose(window)) {
                loop();
            }

            
            
            closing = true;
            thread.join();
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
        Terrain* terrain;

        Interface interface;

        PlayerWidget playerWidget;
        ToolbarWidget toolbarWidget;
        InventoryWidget inventoryWidget;
        ItemSlotWidget itemSlotWidget;
        ItemSlotWidget clearItemSlotWidget;
        TextWidget fpsText;

        FurnaceWidget furnaceWidget;

        Font font;

        PickaxeTool pickaxe;


        Material terrainMaterial = Material::none;


        Recipe makeAluminumPlate = Recipe(ItemStack(registry.getItem("tin_plate"),1));
        Recipe makeFurnace = Recipe(ItemStack(registry.getItem("furnace_item"),1));

        Skybox skybox;

        std::atomic<bool> closing = false;
        

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

            auto playerPrototype = Character::makeDefaultPrototype();

            SkyboxMaterialData skyboxMaterial;
            skyboxMaterial.top = registry.getTexture("space_up");
            skyboxMaterial.bottom = registry.getTexture("space_dn");
            skyboxMaterial.left = registry.getTexture("space_lf");
            skyboxMaterial.right = registry.getTexture("space_rt");
            skyboxMaterial.front = registry.getTexture("space_ft");
            skyboxMaterial.back = registry.getTexture("space_bk");

            skybox.loadResources(*vulkan,skyboxMaterial);

            // auto cubePrototype = Actor::makeDefaultPrototype();
            // cubePrototype->model = registry.getModel("block");
            // cubePrototype->material = registry.getMaterial("tin_block");

            // world.spawn(Actor::makeInstance(cubePrototype.get()));
            

            //world.spawn(Construction::makeInstance(tin,vec3(0)));

            VkPipeline terrainPipeline = vulkan->createManagedPipeline<TerrainVertex>(Vulkan::vertCodePath("terrain"),Vulkan::fragCodePath("terrain"));
            terrainMaterial = vulkan->createMaterial(terrainPipeline,LitMaterialData(registry.getTexture("rock")));
            
            GenerationSettings settings;
            settings.noiseScale = 100;
            settings.stoneType.item = registry.getItem("stone");
            settings.stoneType.texture = registry.getTexture("rock");
            settings.meshBuffer = registry.getModel("block")->meshBuffer;

            terrain = world.spawn(Terrain::makeInstance(terrainMaterial,settings,vec3(0,0,0)));


            // terrain->terrainTypes[0].item = registry.getItem("stone");
            // terrain->terrainTypes[0].texture = registry.getTexture("rock");
            // terrain->terrainTypes[1].item = registry.getItem("tin_ore");
            // terrain->terrainTypes[1].texture = registry.getTexture("tin_ore");
            // terrain->terrainTypes[2].item = registry.getItem("tin_ore");
            // terrain->terrainTypes[2].texture = registry.getTexture("coal_ore");

            makeAluminumPlate.result = ItemStack(registry.getItem("tin_plate"),1);

            makeAluminumPlate.ingredients.push_back(ItemStack(registry.getItem("tin_ore"),5));
            makeAluminumPlate.time = 10;

            makeFurnace.result = ItemStack(registry.getItem("furnace"),1);

            makeFurnace.ingredients.push_back(ItemStack(registry.getItem("stone"),10));
            makeFurnace.time = 3;
            

            player = world.spawn(Character::makeInstance(playerPrototype.get(),vec3(0.0,30.0,0.0)));
            player->model = registry.getModel("capsule_thin");
            player->material = registry.getMaterial("player");

            glfwPollEvents();
            input.clearInputBuffers(); // reset mouse position;

            Debug::loadRenderResources(*vulkan);
            interface.loadRenderResources(*vulkan);


            toolbarWidget.solidSprite = registry.getSprite("solid");


            inventoryWidget.solid = registry.getSprite("solid");
            inventoryWidget.font = &font;
            inventoryWidget.tooltipTextTitle.font = &font;

            furnaceWidget.solid = registry.getSprite("solid");
            furnaceWidget.font = &font;
            furnaceWidget.tooltipTextTitle.font = &font;

            itemSlotWidget.sprite = registry.getSprite("solid");
            itemSlotWidget.font = &font;
            itemSlotWidget.color = Color(0.1,0.1,0.1);

            clearItemSlotWidget.sprite = registry.getSprite("solid");
            clearItemSlotWidget.font = &font;
            clearItemSlotWidget.color = Color::clear;

            inventoryWidget.itemSlot = &itemSlotWidget;
            furnaceWidget.itemSlot = &itemSlotWidget;
            toolbarWidget.itemSlot = &clearItemSlotWidget;
            
            playerWidget.inventoryWidget = &inventoryWidget;
            playerWidget.toolbarWidget = &toolbarWidget;


            font.texture = registry.getTexture("characters");
            font.start = '0';
            font.charSize = vec2(8,12);
            font.textureSize = vec2(312,12);
            font.characters = "0123456789x.ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

            fpsText.font = &font;

            player->recipes.push_back(&makeFurnace);
            
            FurnaceBlock* furnace = static_cast<FurnaceBlock*>(registry.getBlock("furnace"));
            furnace->recipes.push_back(&makeAluminumPlate);
            furnace->widget = &furnaceWidget;

            player->widget = &playerWidget;

            lua["player"] = player;

            lua.do_file("scripts/start.lua");

            
            

        }

        void loop() 
        {

            ZoneScoped;

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

            //std::cout << "starting world frame" << std::endl;
            
            world.frame(vulkan,dt);

            //std::cout << "ending world frame" << std::endl;

            DrawContext drawContext(interface,*vulkan,input);
            Rect screenRect = Rect(drawContext.getScreenSize());

            // cursor
            Sprite solidSprite = registry.getSprite("solid");
            interface.drawRect(*vulkan,Rect::anchored(Rect::centered(vec2(4,0.5)),screenRect,vec2(0.5,0.5)),Color::white,solidSprite);
            interface.drawRect(*vulkan,Rect::anchored(Rect::centered(vec2(0.5,4)),screenRect,vec2(0.5,0.5)),Color::white,solidSprite);

            if(player->widget != nullptr) {
                player->widget->draw(drawContext,*player);
            }

            fpsText.draw(drawContext,vec2(0),std::to_string(1.0f/dt));

            
            if(player->inMenu) 
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } 
            else 
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }

            skybox.addRenderables(*vulkan,camera);

            if(input.getKeyPressed(GLFW_KEY_R)) {
                terrain->loadChunks(player->position,3,vulkan);
            }

            // auto hitOpt = world.raycast(player->getLookRay(),10);
            // if(hitOpt) {
            //     auto hit = hitOpt.value();
                
            //     Debug::drawRay(hit.hit.point,hit.hit.normal,Color::green);
            // }

            // auto hit = Physics::intersectCapsuleBox(player->position,player->getEyePosition(),player->radius,vec3(0),vec3(0.5f));
            // if(hit) {
            //     Physics::resolveBasic(player->position,hit.value());
            // }

            

            //Debug::addRenderables(*vulkan);
            
            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            Debug::clearDebugShapes();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            FrameMark;
            

        }

        void chunkTask() {
            while(!closing) {
                terrain->loadChunks(player->position,1,vulkan);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        

        

        
};