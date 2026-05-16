#define GLM_ENABLE_EXPERIMENTAL
#include "game-application.hpp"

void GameApplication::spawnPlayer(vec3 pos) {
    auto actorPrototype = registry.getActor("player");
    auto playerPrototype = dynamic_cast<Character*>(actorPrototype);

    if(playerPrototype != nullptr) {
        playerID = world->spawn(Character::makeInstance(playerPrototype,pos))->id;
    }
}

void GameApplication::spawnAsteroidScene()  {
    spawnPlayer(vec3(0,50,0));

    std::minstd_rand rnd;

    rnd.seed(0);

    for (size_t i = 0; i < 10; i++)
    {
        std::uniform_real_distribution<> dist(-3000, 3000);
        vec3 postion = vec3(dist(rnd),dist(rnd),dist(rnd));
        std::cout << "spawning terrain at position " << StringHelper::toString(postion) << std::endl;
        auto terrain = world->spawn(Terrain::makeInstance(terrainMaterial,settings,rnd(),postion));
        terrainLoader.addTerrain(terrain->id);
    }

    auto terrain = world->spawn(Terrain::makeInstance(terrainMaterial,settings,rnd(),vec3(0,0,0)));
    terrainLoader.addTerrain(terrain->id);
}

void GameApplication::setup() {
    
    lua = sol::state();
            // Load
    lua.open_libraries(sol::lib::base, sol::lib::package);
    API::loadAPIAll(lua);
    
    // load from files and lua scripts
    loader.loadAll(registry,lua,vulkan);

    input.clearInputBuffers(); // reset mouse position;

    Debug::loadRenderResources(*vulkan);
    interface.loadRenderResources(*vulkan);

    Debug::setLogInfoEnabled(true);

    lastTime = (float)glfwGetTime();

    SkyboxMaterialData skyboxMaterial;
    skyboxMaterial.top = registry.getTexture("space_up");
    skyboxMaterial.bottom = registry.getTexture("space_dn");
    skyboxMaterial.left = registry.getTexture("space_lf");
    skyboxMaterial.right = registry.getTexture("space_rt");
    skyboxMaterial.front = registry.getTexture("space_ft");
    skyboxMaterial.back = registry.getTexture("space_bk");

    skybox.loadResources(*vulkan,skyboxMaterial);

    // prototypes

    auto playerPrototype = registry.addActor<Character>("player");

    // terrain setup

    terrainMaterial = vulkan->createMaterial<LitMaterialData,TerrainVertex>("terrain",LitMaterialData(registry.getTexture("rock")));

    
            
    settings.generationSettings.noiseScale = 1;
    settings.generationSettings.radius = 40;
    settings.generationSettings.noiseFactor = 15;
    settings.generationSettings.noiseOctaves = 5;
    settings.generationSettings.noiseGain = 0.3f;
    settings.generationSettings.noiseLacunarity = 2.5f;
    settings.generationSettings.stoneType.item = registry.getItem("stone");
    settings.generationSettings.stoneType.texture = registry.getTexture("rock");
    settings.generationSettings.oreType.item = registry.getItem("tin_ore");
    settings.generationSettings.oreType.texture = registry.getTexture("tin_ore");

    
    playerPrototype->model = registry.getModel("capsule");
    playerPrototype->material = registry.getMaterial("player");
    registry.addRecipesToVector(playerPrototype->recipes,"crafting",1);
    
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
    playerWidget.speedText = registry.getWidget<TextWidget>("text_default");

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

    //registry.addMaterial("shadow_test",vulkan->createMaterial<UIMaterialData,UIVertex>("shadow_test",UIMaterialData(),options));

    //physicsActor = world.spawn(RigidbodyActor::makeInstance(physicsPrototype,player->getEyePosition(),player->getEyeRotation(),player->getEyeDirection()*20.0f,vec3(0.0f)));

    world = std::make_unique<World>();

    world->constructionMaterial = vulkan->createMaterial<LitMaterialData,ConstructionVertex>("construction",LitMaterialData(registry.getTexture("rock")));

    //spawnAsteroidScene();
    spawnPlayer();

    lua["world"] = world.get();

    // lua
    lua["player"] = world->getActor<Character>(playerID);

    lua.do_file("scripts/start.lua");

}

void GameApplication::debugUI(float dt) {
    
    if(input.getKeyPressed(GLFW_KEY_F1)) {
        debugUIOpen = !debugUIOpen;
    }


    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();

    if(!debugUIOpen) return;
    //imgui commands

    auto player = world->getActor<Character>(playerID);

    ImGui::Begin("Info");
        ImGui::Text("FPS: %8.1f",1.0f/dt);
        if(player != nullptr) {
            auto playerPos = player->getPosition();
            ImGui::Text("position: <%.1f,%.1f,%.1f>",playerPos.x,playerPos.y,playerPos.z);
        }
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

void GameApplication::reload() {
    
    if(world == nullptr) {
        std::cout << "world is null" << std::endl;
        return;
    }

    auto save = world->save();
    terrainLoader.stop();
    terrainLoader.clear();
    world->clear();
    terrainLoader.start(world.get());

    registry.clearDataAssets();

    vulkan->waitIdle(); // we can probably change this later, but for now its fine. just makes sure we finish using all the resources we have before they potentially get modified
    setup();
    DataLoaderImpl dataLoader(registry,world->constructionMaterial);
    world->load(save,dataLoader);
    playerID = world->getActorOfType<Character>();
    lua["player"] = playerID;

}

void GameApplication::loop() {


    if(world == nullptr) {
        std::cout << "world is null" << std::endl;
        return;
    }

    // just for showing the framerate
    float averageTime = 0;
    for (size_t i = 0; i < 60; i++)
    {
        averageTime += frametimes[i];
    }
    averageTime /= 60.0f;


    ZoneScoped;

    // if(input.getKeyPressed(GLFW_KEY_F5)) { // work in progress
    //     reload();
    // }
    

    // save and load 
    if(input.getKeyPressed(GLFW_KEY_F6)) {
        std::cout << "saving" << std::endl;
        auto data = world->save();
        SaveHelper::save<data_World>(data,"world.dat");
    }
    
    
    if(input.getKeyPressed(GLFW_KEY_F7)) {
        std::cout << "loading" << std::endl;
        auto dataOpt = SaveHelper::load<data_World>("world.dat");
        if(dataOpt) {
            playerID = Invalid_ActorID;
            DataLoaderImpl dataLoader(registry,world->constructionMaterial);
            world->load(dataOpt.value(),dataLoader);
            playerID = world->getActorOfType<Character>();
            lua["player"] = playerID;
        }
    }

    auto player = world->getActor<Character>(playerID);

    auto& camera = world->getCamera();

    ivec2 frameSize = window->getFrameBufferSize();

    // Get time
    float dt = (float)glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    camera.setAspect(frameSize.x,frameSize.y);
    
    // test inputs
    //camera.rotate(vec3(mouseDelta.y * dt,mouseDelta.x * dt,0));
    camera.rotate(vec3(0,dt*-10,0));

    terrainLoader.setCameraPosition(camera.position);

    if(player != nullptr) {
        player->alwaysRender = false;
        player->setCamera(camera,world->getInterpolationTime());
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

    

    if(player != nullptr) {
        if(player->inMenu) 
        {
            window->setCursorMode(CursorMode::Normal);
        } 
        else 
        {
            window->setCursorMode(CursorMode::Locked);
            drawContext.disableClicks();
        }
        playerWidget.draw(drawContext,*player);
    }
    

    fpsText.draw(drawContext,vec2(0),std::to_string(1.0f/averageTime));

    
    //interface.drawRect(*vulkan,Rect(5,5,300,300),Color::white,Sprite(0),registry.getMaterial("shadow_test"));
    

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