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
#include "helper/random.hpp"
#include "graphics/camera.hpp"
#include "graphics/skybox.hpp"
#include "world.hpp"
#include "actor/actors-all.hpp"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "sol/sol.hpp"
#include "interface/interface.hpp"
#include "helper/clock.hpp"
#include "terrain-loader.hpp"
#include <thread>
#include <chrono>

// #include <tracy/Tracy.hpp>
// #include <tracy/TracyVulkan.hpp>

#include "interface/actor/player-widget.hpp"
#include "interface/block/furnace-widget.hpp"

#include "item/items-all.hpp"
#include "item/recipe.hpp"

#include "physics/jolt-layers.hpp"


#include "imgui/imgui.h"
#include "persistance/data-world.hpp"
#include "persistance/save-helper.hpp"
#include "persistance/data-loader-impl.hpp"

#include "interface/debug/cheats-menu.hpp"

JPH_SUPPRESS_WARNINGS

using std::string;
using std::unique_ptr, std::shared_ptr;


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

            setup();

            terrainLoader.start();
            //terrain->loadChunks(&world,player->getPosition(),2,vulkan);
            

            while (!glfwWindowShouldClose(window)) {
                loop();

                // if(input.getKey(GLFW_KEY_ESCAPE)) {
                //     break;
                // }
            }

            
            terrainLoader.stop();
            vulkan->waitIdle();

        }

        
        
        private:

        std::unique_ptr<World> world = nullptr;
        
        std::vector<std::thread> chunkWorkers;

        Registry registry;

        Loader loader;
        
        sol::state lua;

        GLFWwindow* window = nullptr;   
        
        Vulkan* vulkan;

        Input input;

        std::vector<float> frameTimes;

        std::shared_ptr<Character> player = nullptr;

        Interface interface;

        PlayerWidget playerWidget;
        ToolbarWidget toolbarWidget;
        InventoryWidget inventoryWidget;
        ItemSlotWidget clearItemSlotWidget;
        TextWidget fpsText;

        FurnaceWidget furnaceWidget;

        Font font;

        TerrainLoader terrainLoader;


        Material terrainMaterial = Material::none;
        Material terrainMaterialDebug = Material::none;


        Recipe makeAluminumPlate = Recipe(ItemStack(registry.getItem("tin_plate"),1));
        Recipe makeFurnace = Recipe(ItemStack(registry.getItem("furnace_item"),1));
        Recipe makeThruster = Recipe(ItemStack(registry.getItem("thruster"),1));
        Recipe makeCockpit = Recipe(ItemStack(registry.getItem("cockpit"),1));
        Recipe makePickaxe = Recipe(ItemStack(registry.getItem("pickaxe"),1));

        Skybox skybox;

        std::atomic<bool> closing = false;
        std::atomic<bool> chunkLoadPaused = false;

        GenerationSettings generationSettings;

        float frametimes[60];
        int currentFrameTimeIndex = 0;

        bool debugUIOpen = false;
        char consoleBuffer[1024] = ""; //temp
        

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
            #if __APPLE__
                input.currentMousePosition = vec2(xposIn,yposIn)* 2.0f; //retina bs. Doesn't actually detect retina so itll probably not work on macs that have no retina
            #else
                input.currentMousePosition = vec2(xposIn,yposIn);
            #endif
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

        void spawnPlayer() {
            auto actorPrototype = registry.getActor("player");
            auto playerPrototype = dynamic_cast<Character*>(actorPrototype);

            if(playerPrototype != nullptr) {
                player = world->spawn(Character::makeInstance(playerPrototype,vec3(0,0,0)));
            }
        }

        void spawnAsteroidScene() {

            
            spawnPlayer();

            std::minstd_rand rnd;

            rnd.seed(0);

            for (size_t i = 0; i < 10; i++)
            {
                std::uniform_real_distribution<> dist(-3000, 3000);
                vec3 postion = vec3(dist(rnd),dist(rnd),dist(rnd));
                std::cout << "spawning terrain at position " << StringHelper::toString(postion) << std::endl;
                auto terrain = world->spawn(Terrain::makeInstance(terrainMaterial,generationSettings,rnd(),postion));
                terrainLoader.addTerrain(terrain);
            }

            auto terrain = world->spawn(Terrain::makeInstance(terrainMaterial,generationSettings,rnd(),vec3(0,0,0)));
            terrainLoader.addTerrain(terrain);
        }

        void spawnLargeAsteroidScene() {

            spawnPlayer();

            auto largeGenerationSettings = generationSettings;
            largeGenerationSettings.radius = 100;
            auto terrain = world->spawn(Terrain::makeInstance(terrainMaterial,largeGenerationSettings,0,vec3(0,-110,0)));
            terrainLoader.addTerrain(terrain);
        }

        void spawnPhysicsScene() {
            spawnPlayer();

            auto physicsPrototype = registry.getActor<RigidbodyActor>("physics");

            if(physicsPrototype != nullptr) {
                world->spawn(RigidbodyActor::makeInstance(physicsPrototype,vec3(0.0f,0.0f,5.0f),quat(vec3(0)),vec3(0.0f),vec3(0.0f)));
                world->spawn(RigidbodyActor::makeInstance(physicsPrototype,vec3(0.0f,0.0f,10.0f),quat(vec3(0)),vec3(0.0f),vec3(0.0f)));
            }
            world->spawn(RigidbodyActor::makeInstance(registry.getActor<RigidbodyActor>("physics_cow"),vec3(0.0f,0.0f,15.0f),quat(vec3(0)),vec3(0.0f),vec3(0.0f)));
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

            Debug::setLogInfoEnabled(true);

            lastTime = (float)glfwGetTime();

            auto prototype = registry.addActor<RigidbodyActor>("physics");
            prototype->model = registry.getModel("cube");
            prototype->material = registry.getMaterial("grid");

            auto prototype_cow = registry.addActor<RigidbodyActor>("physics_cow");
            prototype_cow->model = registry.getModel("cube");
            prototype_cow->material = registry.getMaterial("cow");

            // prototypes

            auto playerPrototype = registry.addActor<Character>("player");

            SkyboxMaterialData skyboxMaterial;
            skyboxMaterial.top = registry.getTexture("space_up");
            skyboxMaterial.bottom = registry.getTexture("space_dn");
            skyboxMaterial.left = registry.getTexture("space_lf");
            skyboxMaterial.right = registry.getTexture("space_rt");
            skyboxMaterial.front = registry.getTexture("space_ft");
            skyboxMaterial.back = registry.getTexture("space_bk");

            skybox.loadResources(*vulkan,skyboxMaterial);

            // terrain setup

            terrainMaterial = vulkan->createMaterial<LitMaterialData,TerrainVertex>("terrain",LitMaterialData(registry.getTexture("rock")));

                    
            generationSettings.noiseScale = 1;
            generationSettings.radius = 40;
            generationSettings.noiseFactor = 15;
            generationSettings.noiseOctaves = 5;
            generationSettings.noiseGain = 0.3f;
            generationSettings.noiseLacunarity = 2.5f;
            generationSettings.stoneType.item = registry.getItem("stone");
            generationSettings.stoneType.texture = registry.getTexture("rock");
            generationSettings.oreType.item = registry.getItem("tin_ore");
            generationSettings.oreType.texture = registry.getTexture("tin_ore");


            
            

    
            
            playerPrototype->model = registry.getModel("capsule_thin");
            playerPrototype->material = registry.getMaterial("player");
            registry.addRecipesToVector(playerPrototype->recipes,"crafting",1);
            // spawn player
            //player = world.spawn(Character::makeInstance(playerPrototype.get(),vec3(50,0,0)));

            
            // UI
            toolbarWidget.itemSlotSprite = registry.getSprite("item_slot");
            toolbarWidget.sprite = registry.getSprite("tech_hotbar");
            toolbarWidget.selectorSprite =  registry.getSprite("tech_hotbar_selector");
            toolbarWidget.selectorSize = 95;
            toolbarWidget.slotSize = 75;
            toolbarWidget.slotHeight = 137;
            toolbarWidget.slotGap = 3;
            

            inventoryWidget.backgroundSprite = registry.getSprite("solid");
            inventoryWidget.font = &font;
            inventoryWidget.tooltipTextTitle = registry.getWidget<TextWidget>("text_default");

            furnaceWidget.solid = registry.getSprite("solid");
            furnaceWidget.font = &font;
            furnaceWidget.tooltipTextTitle = registry.getWidget<TextWidget>("text_default");
            furnaceWidget.recipeSlot = registry.getWidget<ItemSlotWidget>("recipe_slot");

            auto itemSlotWidget = registry.getWidget<ItemSlotWidget>("item_slot");

            clearItemSlotWidget.sprite = registry.getSprite("solid");
            clearItemSlotWidget.font = &font;
            clearItemSlotWidget.color = Color::clear;

            inventoryWidget.itemSlot = itemSlotWidget;
            inventoryWidget.recipeSlot = registry.getWidget<ItemSlotWidget>("recipe_slot");
            furnaceWidget.itemSlot = itemSlotWidget;
            toolbarWidget.itemSlot = registry.getWidget<ItemSlotWidget>("toolbar_item_slot");
            
            playerWidget.inventoryWidget = &inventoryWidget;
            playerWidget.toolbarWidget = &toolbarWidget;
            playerWidget.cursorSlotWidget = registry.getWidget<ItemSlotWidget>("toolbar_item_slot");
            playerWidget.cursorRectSprite = registry.getSprite("solid");

            PipelineOptions options;
            options.blend = VK_TRUE;
            LitMaterialData materialData;

            materialData.texture = registry.getTexture("rock");
            auto particleMaterial = vulkan->createMaterial<LitMaterialData,Vertex>("lit",materialData,options);


            fpsText.font = &font;

            // recipes
            
            FurnaceBlock* furnace = static_cast<FurnaceBlock*>(registry.getBlock("furnace"));
            registry.addRecipesToVector(furnace->recipes,"smelting",1);
            furnace->widget = &furnaceWidget;

            playerPrototype->widget = &playerWidget;
            // wowie
            auto effectPrototype = registry.addActor<ParticleEffectActor>("effect");
            auto& effect = effectPrototype->effect;
            effect.spawnRate = 0;
            effect.initialSpawnCount = 20;
            effect.initialVelocity = {0.0f,5.0f};
            effect.emitterShape.radius = 0.5f;
            effect.mesh = registry.getModel("rock_shard");
            effect.material = particleMaterial;
            effect.lifeTime = {0.5f,1.0f};
            effect.particleSize = {0.2f,0.0f};
            effect.initialAngularVelocity = {0.0f,90.0f};

            auto pickaxe = dynamic_cast<PickaxeTool*>(registry.getItem("pickaxe")); 
            pickaxe->testEffect = effectPrototype;

            options = {};
            options.depthTestEnabled = VK_TRUE;
            options.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            options.blend = VK_TRUE;

            registry.addMaterial("shadow_test",vulkan->createMaterial<UIMaterialData,UIVertex>("shadow_test",UIMaterialData(),options));

            //physicsActor = world.spawn(RigidbodyActor::makeInstance(physicsPrototype,player->getEyePosition(),player->getEyeRotation(),player->getEyeDirection()*20.0f,vec3(0.0f)));

            world = std::make_unique<World>();

            world->constructionMaterial = vulkan->createMaterial<LitMaterialData,ConstructionVertex>("construction",LitMaterialData(registry.getTexture("rock")));

            spawnLargeAsteroidScene();

            lua["world"] = world.get();

            // lua
            lua["player"] = player;

            lua.do_file("scripts/start.lua");

            
        }

        void debugUI(float dt) {
            
            if(input.getKeyPressed(GLFW_KEY_F1)) {
                debugUIOpen = !debugUIOpen;
            }

            
            

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();

            ImGui::NewFrame();

            if(!debugUIOpen) return;
            //imgui commands

            ImGui::Begin("Info");
                ImGui::Text("FPS: %8.1f",1.0f/dt);
                auto playerPos = player->getPosition();
                ImGui::Text("position: <%.1f,%.1f,%.1f>",playerPos.x,playerPos.y,playerPos.z);
                auto playerAngularVelocity = player->body.getAngularVelocity();
                ImGui::Text("angular velocity: <%.5f,%.5f,%.5f>",playerAngularVelocity.x,playerAngularVelocity.y,playerAngularVelocity.z);
            ImGui::End();

            
            ImGui::Begin("Console");
                if(ImGui::InputText("##",consoleBuffer,IM_ARRAYSIZE(consoleBuffer),ImGuiInputTextFlags_EnterReturnsTrue)) {
                    Debug::lua("> " + (string)consoleBuffer);
                    try {
                        auto result = lua.do_string("return " + (string)consoleBuffer);
                        lua["print"](result);
                    } catch(...) {
                        lua.do_string(consoleBuffer);
                    }

                }
                auto consoleLines = Debug::getluaConsole();
                for(int i = 1;i < 50;i++) {
                    int index = static_cast<int>(consoleLines.size())-i;
                    if(index < 0) break;
                    auto& line = consoleLines[index];
                    ImGui::Text(line.c_str());
                }
            ImGui::End();

            ImGui::Begin("TerrainLoader");

                for (int i = 0; i < TerrainLoader::terrainJobCount; i++)
                {
                    ImGui::PushID(i);
                    std::ostringstream stream;
                    switch(terrainLoader.getJobState(i)) {
                        case TerrainJobState::FINISHED:
                            ImGui::Text("FINISHED");
                            break;
                        case TerrainJobState::IN_PROGRESS:
                            stream << "IN PROGRESS:" << terrainLoader.getJobWorker(i);
                            ImGui::Text(stream.str().c_str());
                            break;
                        case TerrainJobState::WAITING:
                            ImGui::Text("FINISHED");
                            break;
                    }
                    ImGui::PopID();
                }
                

            ImGui::End();

            if(player != nullptr && world != nullptr) {
                DebugMenu::cheatsMenu(*player,*world,registry);
            }

            ImGui::ShowDemoWindow();


        }

        void loop() 
        {

            if(world == nullptr) {
                std::cout << "world is null" << std::endl;
                return;
            }
            float averageTime = 0;
            for (size_t i = 0; i < 60; i++)
            {
                averageTime += frametimes[i];
            }
            averageTime /= 60.0f;


            //ZoneScoped;
            
            // Get inputs, window resize, and more
            glfwPollEvents();
            
            if(input.getKeyPressed(GLFW_KEY_F6)) {
                std::cout << "saving" << std::endl;
                auto data = world->save();
                SaveHelper::save<data_World>(data,"world.dat");
            }
            
            if(input.getKeyPressed(GLFW_KEY_F7)) {
                std::cout << "loading" << std::endl;
                auto dataOpt = SaveHelper::load<data_World>("world.dat");
                if(dataOpt) {
                    player = nullptr;
                    DataLoaderImpl dataLoader(registry,world->constructionMaterial);
                    world->load(dataOpt.value(),dataLoader);
                    player = world->getActorOfType<Character>();
                    lua["player"] = player;
                }
            }

            auto& camera = world->getCamera();

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

            terrainLoader.setCameraPosition(camera.position);

            if(player != nullptr) {
                player->setCamera(camera);
                player->processInput(input);
            }

            frametimes[currentFrameTimeIndex] = dt;
            currentFrameTimeIndex++;
            if(currentFrameTimeIndex >= 60) {
                currentFrameTimeIndex = 0;
            }
            
            // vulkan->mainLight.direction = vulkan->mainLight.direction * glm::quat(glm::radians(vec3(0,dt*90,0)));
            // std::cout << StringHelper::toString(vulkan->mainLight.direction) << std::endl;
            //std::cout << StringHelper::toString(player->getPosition()) << std::endl;

            
            world->frame(vulkan,dt);
            skybox.addRenderables(*vulkan,camera); // draw before UI

            //std::cout << "ending world frame" << std::endl;

            DrawContext drawContext(interface,*vulkan,input);
            Rect screenRect = Rect(drawContext.getScreenSize());

            // cursor
            Sprite solidSprite = registry.getSprite("solid");
            interface.drawRect(*vulkan,Rect::anchored(Rect::centered(vec2(4,0.5)),screenRect,vec2(0.5,0.5)),Color::white,solidSprite);
            interface.drawRect(*vulkan,Rect::anchored(Rect::centered(vec2(0.5,4)),screenRect,vec2(0.5,0.5)),Color::white,solidSprite);

            if(player != nullptr) {
                playerWidget.draw(drawContext,*player);
                if(player->inMenu) 
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                } 
                else 
                {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
            }
            

            fpsText.draw(drawContext,vec2(0),std::to_string(1.0f/averageTime));

            
        
            

            vulkan->mainLight.direction = glm::quat(glm::radians(vec3(0,1 * dt,0))) * vulkan->mainLight.direction;

            Debug::addRenderables(*vulkan);

            debugUI(dt); 
            
            // do all the end of frame code in vulkan
            vulkan->render(camera);
            vulkan->clearObjects();

            Debug::clearDebugShapes();

            //std::cout << (int)(renderClock.reset() * 1000) << "ms" << std::endl;

            input.clearInputBuffers();

            //FrameMark;
            

        }

        

        

        
};