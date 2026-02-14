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

// #include <tracy/Tracy.hpp>
// #include <tracy/TracyVulkan.hpp>

#include "interface/actor/player-widget.hpp"
#include "interface/block/furnace-widget.hpp"

#include "item/items-all.hpp"
#include "item/recipe.hpp"

#include "physics/jolt-layers.hpp"



JPH_SUPPRESS_WARNINGS

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
        uint32_t windowWidth = 1280;
        uint32_t windowHeight = 720;
        void run() {

            std::cout << "main thread id:" << std::this_thread::get_id() << std::endl;

            setup();

            std::thread thread(&GameApplication::chunkTask,this);
            terrain->loadChunks(&world,player->getPosition(),2,vulkan);
            

            while (!glfwWindowShouldClose(window)) {
                loop();

                if(input.getKey(GLFW_KEY_ESCAPE)) {
                    break;
                }
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

        RigidbodyActor* physicsPrototype;

        RigidbodyActor* physicsActor;


        Material terrainMaterial = Material::none;
        Material terrainMaterialDebug = Material::none;


        Recipe makeAluminumPlate = Recipe(ItemStack(registry.getItem("tin_plate"),1));
        Recipe makeFurnace = Recipe(ItemStack(registry.getItem("furnace_item"),1));
        Recipe makeThruster = Recipe(ItemStack(registry.getItem("thruster"),1));
        Recipe makeCockpit = Recipe(ItemStack(registry.getItem("cockpit"),1));

        Skybox skybox;

        std::atomic<bool> closing = false;
        std::atomic<bool> chunkLoadPaused = false;

        float frametimes[60];
        int currentFrameTimeIndex = 0;
        

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
            
            // load from files and lua scripts
            loader.loadAll(registry,lua,vulkan);

            glfwPollEvents();
            input.clearInputBuffers(); // reset mouse position;

            Debug::loadRenderResources(*vulkan);
            interface.loadRenderResources(*vulkan);

            Debug::setLogInfoEnabled(false);

            lastTime = (float)glfwGetTime();

            // prototypes

            auto playerPrototype = Character::makeDefaultPrototype();

            SkyboxMaterialData skyboxMaterial;
            skyboxMaterial.top = registry.getTexture("space_up");
            skyboxMaterial.bottom = registry.getTexture("space_dn");
            skyboxMaterial.left = registry.getTexture("space_lf");
            skyboxMaterial.right = registry.getTexture("space_rt");
            skyboxMaterial.front = registry.getTexture("space_ft");
            skyboxMaterial.back = registry.getTexture("space_bk");

            skybox.loadResources(*vulkan,skyboxMaterial);

            auto planePrototype = Actor::makeDefaultPrototype();
            planePrototype->model = registry.getModel("plane");
            planePrototype->material = registry.getMaterial("grid");

            //world.spawn(Actor::makeInstance(planePrototype.get(),vec3(0,0,0)));

            // terrain setup

            VkPipeline terrainPipeline = vulkan->createManagedPipeline<TerrainVertex>(Vulkan::vertCodePath("terrain"),Vulkan::fragCodePath("terrain"));
            terrainMaterial = vulkan->createMaterial(terrainPipeline,LitMaterialData(registry.getTexture("rock")));

            VkPipeline constructionPipeline = vulkan->createManagedPipeline<ConstructionVertex>(Vulkan::vertCodePath("construction"),Vulkan::fragCodePath("construction"));
            world.constructionMaterial = vulkan->createMaterial(constructionPipeline,LitMaterialData(registry.getTexture("rock")));
            
            GenerationSettings settings;
            settings.noiseScale = 100;
            settings.radius = 50;
            settings.stoneType.item = registry.getItem("stone");
            settings.stoneType.texture = registry.getTexture("rock");
            settings.oreType.item = registry.getItem("tin_ore");
            settings.oreType.texture = registry.getTexture("tin_ore");

            terrain = world.spawn(Terrain::makeInstance(terrainMaterial,settings,vec3(0,0,0)));

            physicsPrototype = registry.addActor<RigidbodyActor>("physics_block");
            
            physicsPrototype->model = registry.getModel("cube");
            physicsPrototype->material = registry.getMaterial("tin_block");


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

            makeThruster.result = ItemStack(registry.getItem("thruster"),1);

            makeThruster.ingredients.push_back(ItemStack(registry.getItem("tin_plate"),5));
            makeThruster.time = 5;

            makeCockpit.result = ItemStack(registry.getItem("cockpit"),1);

            makeCockpit.ingredients.push_back(ItemStack(registry.getItem("tin_plate"),3));
            makeCockpit.time = 5;
            
            // spawn player
            player = world.spawn(Character::makeInstance(playerPrototype.get(),vec3(3.9,1.4,26.1)));
            player->model = registry.getModel("capsule_thin");
            player->material = registry.getMaterial("player");

            
            // UI
            toolbarWidget.solidSprite = registry.getSprite("item_slot");


            inventoryWidget.solid = registry.getSprite("solid");
            inventoryWidget.font = &font;
            inventoryWidget.tooltipTextTitle.font = &font;

            furnaceWidget.solid = registry.getSprite("solid");
            furnaceWidget.font = &font;
            furnaceWidget.tooltipTextTitle.font = &font;

            itemSlotWidget.sprite = registry.getSprite("item_slot");
            itemSlotWidget.font = &font;
            itemSlotWidget.color = Color(0.2,0.2,0.2);

            clearItemSlotWidget.sprite = registry.getSprite("solid");
            clearItemSlotWidget.font = &font;
            clearItemSlotWidget.color = Color::clear;

            inventoryWidget.itemSlot = &itemSlotWidget;
            furnaceWidget.itemSlot = &itemSlotWidget;
            toolbarWidget.itemSlot = &itemSlotWidget;
            
            playerWidget.inventoryWidget = &inventoryWidget;
            playerWidget.toolbarWidget = &toolbarWidget;


            font.texture = registry.getTexture("characters");
            font.start = '0';
            font.charSize = vec2(8,12);
            font.textureSize = vec2(312,12);
            font.characters = "0123456789x.ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

            fpsText.font = &font;

            // recipes
            player->recipes.push_back(&makeFurnace);
            player->recipes.push_back(&makeCockpit);
            player->recipes.push_back(&makeThruster);
            
            FurnaceBlock* furnace = static_cast<FurnaceBlock*>(registry.getBlock("furnace"));
            furnace->recipes.push_back(&makeAluminumPlate);
            furnace->widget = &furnaceWidget;

            player->widget = &playerWidget;
            // wowie
            auto effectPrototype = registry.addActor<ParticleEffectActor>("effect");
            auto& effect = effectPrototype->effect;
            effect.spawnRate = 10;
            effect.initialVelocity = 1;
            effect.mesh = registry.getModel("quad");
            effect.material = registry.getMaterial("grid");
            effect.lifeTime = 5;
            
            world.spawn(ParticleEffectActor::makeInstance(effectPrototype,player->getPosition()));

            //physicsActor = world.spawn(RigidbodyActor::makeInstance(physicsPrototype,player->getEyePosition(),player->getEyeRotation(),player->getEyeDirection()*20.0f,vec3(0.0f)));

            // lua
            lua["player"] = player;

            lua.do_file("scripts/start.lua");

        }

        void loop() 
        {

            //ZoneScoped;

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

            frametimes[currentFrameTimeIndex] = dt;
            currentFrameTimeIndex++;
            if(currentFrameTimeIndex >= 60) {
                currentFrameTimeIndex = 0;
            }

            float averageTime = 0;
            for (size_t i = 0; i < 60; i++)
            {
                averageTime += frametimes[i];
            }
            averageTime /= 60.0f;
            

            //std::cout << "starting world frame" << std::endl;
            //std::cout << StringHelper::toString(player->getPosition()) << std::endl;
            
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

            fpsText.draw(drawContext,vec2(0),std::to_string(1.0f/averageTime));

            
            if(player->inMenu) 
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            } 
            else 
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }


            skybox.addRenderables(*vulkan,camera);

            // if(input.getKeyPressed(GLFW_KEY_R)) {
            //     terrain->loadChunks(player->position,3,vulkan);
            // }
            // if(input.getKeyPressed(GLFW_KEY_G)) {
            //     auto hitOpt = world.raycast(player->getLookRay(),100);
            //     if(hitOpt) {
            //         auto hit = hitOpt.value();

            //         auto terrain =  dynamic_cast<Terrain*>(hit.actor);
            //         if(terrain != nullptr) {
            //             terrain->selectedChunk = hit.component;
            //             std::cout << terrain->getDebugInfo(hit.component) << std::endl;
            //         }
                    
            //         Debug::drawRay(hit.hit.point,hit.hit.normal,Color::green);
            //         Debug::drawPoint(hit.hit.point);
            //     }
            // }

            if(input.getKeyPressed(GLFW_KEY_R)) {
                world.spawn(RigidbodyActor::makeInstance(physicsPrototype,player->getEyePosition()+player->getEyeDirection()*2.0f,player->getRotation(),player->getEyeDirection()*20.0f,vec3(0)));
                // physicsActor->setPosition(player->getEyePosition());
                // physicsActor->setRotation(player->getEyeRotation());
                // physicsActor->velocity = (player->getEyeDirection()*20.0f);
                // physicsActor->angularVelocity = vec3(0);
            }

            if(input.getKeyPressed(GLFW_KEY_F2)) {
                world.pausePhysics = !world.pausePhysics;
            }
            if(input.getKeyPressed(GLFW_KEY_RIGHT)) {
                world.stepPhysics = true;
            }

            // auto hit = Physics::intersectCapsuleBox(player->position,player->getEyePosition(),player->radius,vec3(0),vec3(0.5f));
            // if(hit) {
            //     Physics::resolveBasic(player->position,hit.value());
            // }
            if(input.getKeyPressed(GLFW_KEY_RIGHT)) {
                chunkLoadPaused = false;
            }
            

            

            Debug::addRenderables(*vulkan);
            
            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            Debug::clearDebugShapes();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            //FrameMark;
            

        }

        void chunkTask() {
            while(!closing) {
                if(!chunkLoadPaused) {
                    terrain->loadChunks(&world,player->getPosition(),1,vulkan);
                }
                //std::cout << "loading chunks" << std::endl;
                //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            
        }

        

        

        
};