#pragma once

#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "client-socket.hpp"
#include "server-socket.hpp"

#include "graphics/vulkan.hpp"
#include "engine/window.hpp"
#include "imgui/imgui.h"
#include "graphics/skybox.hpp"
// #include <tracy/Tracy.hpp>
#include "cista.h"

#include "engine/loader.hpp"

#include "persistance/data-loader-impl.hpp"

#include "networking/message/message-actor.hpp"
#include "networking/message/message-character.hpp"

#include "networking/listener/network-world-listener.hpp"
#include "networking/listener/network-character-listener.hpp"
#include "networking/listener/network-construction-listener.hpp"

#include "actor/character.hpp"
#include "interface/debug/issues-menu.hpp"

#include <string>

#include <iostream>

#include <format>
#include <functional>

class NetTestApp
{

    enum class AppState
    {
        MainMenu,
        Host,
        Client
    };

    const uint16 nPort = 27020;

    Window *window;

    Vulkan *vulkan;

    Loader loader;

    sol::state lua;

    Registry registry;
    Clock clock;

    ServerSocket server;
    ClientSocket client;

    Skybox skybox;

    Interface interface;

    World world;

    float actorUpdateDt = {};

    bool mouseControl = false;

    ActorID playerID = Invalid_ActorID;

    AppState appState = AppState::MainMenu;

    char addressBuffer[1024] = "127.0.0.1";
    char messageBuffer[1024] = "";
    char nameBuffer[1024] = "user";

    
    NetworkWorldListener worldListener;

    NetworkCharacterListener clientCharacterListener;
     

    std::map<ActorID,ConnectedClient> actorOwnershipMap;

    std::vector<string> chatLog;

    void sendActorToClient(Actor *actor, const ConnectedClient client,bool isLocalPlayer = false)
    {
        auto dataOpt = actor->getDataEntry();
        if(dataOpt) {
            server.sendMessageToClient(client, MessageSpawnActor(dataOpt.value(),isLocalPlayer));
        }
    }

    void sendActorUpdates()
    {
        for (auto *actor : world.getActors())
        {
            if (actor->networkLocal)
            {
                auto message = MessageUpdateActorTransform(actor->id);
                message.newPosition.set(actor->getPosition());
                message.newRotation.set(actor->getRotation());
                server.sendMessageToAllClients(message);
            }
        }
    }

    void sendActorUpdateClient()
    {
        auto actor = world.getActor<Actor>(playerID);
        if (actor != nullptr)
        {
            auto message = MessageUpdateActorTransform(actor->id);
            message.newPosition.set(actor->getPosition());
            message.newRotation.set(actor->getRotation());
            client.sendMessage(message);
        }
    }

    void receiveActorUpdate(MessageUpdateActorTransform contents)
    {
        auto actor = world.getActor<Actor>(contents.id);

        if (actor != nullptr)
        {
            actor->setPosition(contents.newPosition.toVec3());
            actor->setRotation(contents.newRotation.toQuat());
            actor->updateLastTransform();
        }
        else
        {
            Debug::warn("couldn't follow actor update... ");
        }
    }

    void setup()
    {

        lua.open_libraries(sol::lib::base, sol::lib::package);
        API::loadAPIAll(lua);

        vulkan->waitIdle();

        interface.loadRenderResources(*vulkan);

        loader.loadAll(registry, lua, vulkan); 

        SkyboxMaterialData skyboxMaterial;
        skyboxMaterial.top = registry.getTexture("space_up");
        skyboxMaterial.bottom = registry.getTexture("space_dn");
        skyboxMaterial.left = registry.getTexture("space_lf");
        skyboxMaterial.right = registry.getTexture("space_rt");
        skyboxMaterial.front = registry.getTexture("space_ft");
        skyboxMaterial.back = registry.getTexture("space_bk");

        skybox.loadResources(*vulkan, skyboxMaterial);

        auto playerWidget = registry.addObject<PlayerWidget>("player_widget");
        playerWidget->inventoryWidget = registry.getPtr<InventoryWidget>("inventory");
        playerWidget->toolbarWidget = registry.getPtr<ToolbarWidget>("toolbar");
        playerWidget->cursorSlotWidget = registry.getPtr<ItemSlotWidget>("toolbar_item_slot");
        playerWidget->cursorRectSprite = registry.getSprite("solid");
        playerWidget->speedText = registry.getPtr<TextWidget>("text_default");

        registry.getActor<Character>("player")->widget = playerWidget;

        worldListener.server = &server; // this needs to happen like first
        worldListener.subscribeAll(world);
        worldListener.registerType<NetworkCharacterListener>(data_ActorType::PLAYER);
        worldListener.registerType<NetworkConstructionListener>(data_ActorType::CONSTRUCTION);

        server.addRawMessageCallback("CHAT", [&](IncomingMessageServer<string> message)
        { 
            serverReceiveMessage(message.client->getName(), message.contents); 
        });

        server.addRawMessageCallback("JOIN", [&](IncomingMessageServer<string> message)
        {
            server.setClientName(message.client, message.contents);
            serverReceiveText(message.contents + " has joined!");
            for (auto *actor : world.getActors())
            {
                sendActorToClient(actor, *message.client);
            }
            auto newPlayer = spawnRemotePlayer(message.client);
            sendActorToClient(newPlayer, *message.client,true);
            server.sendMessageToAllClientsExcept(message.client, MessageSpawnActor(newPlayer->getDataEntry().value(),false));
        });

        server.addRawMessageCallback("LEAV", [&](IncomingMessageServer<string> message)
                                     {
            serverReceiveText(message.client->getName() + " has left");
            if(message.client->actorID != Invalid_ActorID) {
                world.destroyActor(message.client->actorID);
            } 
        });
        
        server.addMessageCallback<MessageCharacterUpdateItemEvent>([&](IncomingMessageServer<MessageCharacterUpdateItemEvent> message)
        { 
            auto character = world.getActor<Character>(message.contents.actor);
            if(character != nullptr) {
                character->setCurrentTool(message.contents.slot);
            } else {
                Debug::warn("incoming message character is nullptr");
            }
            server.sendMessageToAllClientsExcept(message.client,message.contents);
        });

        server.addMessageCallback<MessageCharacterDropItemEvent>([&](IncomingMessageServer<MessageCharacterDropItemEvent> message)
        { 
            auto character = world.getActor<Character>(message.contents.actor);
            if(character != nullptr) {
                ItemStack stack;
                DataLoaderImpl dataLoader(registry,world.constructionMaterial);
                stack.load(message.contents.stack,dataLoader);
                character->take(stack);
                world.spawn(ItemActor::makeInstance(stack,message.contents.position.toVec3()));
            } else {
                Debug::warn("incoming message character is nullptr");
            }
            server.sendMessageToAllClientsExcept(message.client,message.contents);
        });

        client.addMessageCallback<MessageSpawnActor>([&](IncomingMessageClient<MessageSpawnActor> message)
        {
            
            DataLoaderImpl dataLoader(registry,world.constructionMaterial);
            auto actor = world.loadActor(message.contents.actorEntry,dataLoader);
            if(actor != nullptr) {
                actor->networkLocal = false;
            } 
            if(message.contents.localPlayer) {
                playerID = actor->id;
                auto player = dynamic_cast<Character*>(actor);
                clientCharacterListener.subscribeClient(*actor);
                clientCharacterListener.messageSender = &client;
                if(player != nullptr) {
                    player->alwaysRender = false;
                } else {
                    Debug::warn("player is not of Character type!");
                }

            }
        });

        client.addMessageCallback<MessageDestroyActor>([&](IncomingMessageClient<MessageDestroyActor> message)
        {
            auto actor = world.getActor<Actor>(message.contents.id);
            if(actor != nullptr) {
                actor->destroy(&world);
            }
            
        });

        client.addMessageCallback<MessageUpdateActorTransform>([&](IncomingMessageClient<MessageUpdateActorTransform> message)
        {
            if(message.contents.id == playerID) return; //just skip for now
            
            receiveActorUpdate(message.contents);
        });

        server.addMessageCallback<MessageUpdateActorTransform>([&](IncomingMessageServer<MessageUpdateActorTransform> message)
        {
            receiveActorUpdate(message.contents);
        });

        client.addMessageCallback<MessageCharacterUpdateItemEvent>([&](IncomingMessageClient<MessageCharacterUpdateItemEvent> message)
        { 
            if(message.contents.actor == playerID) return; //ignore for now

            auto character = world.getActor<Character>(message.contents.actor);
            if(character != nullptr) {
                character->setToolbar(message.contents.slot,ItemStack(registry.getItem((string)message.contents.itemName),1));
                character->setCurrentTool(message.contents.slot);
            } else {
                Debug::warn("incoming message character is nullptr");
            }
        });

        client.addMessageCallback<MessageCharacterInventoryChangeEvent>([&](IncomingMessageClient<MessageCharacterInventoryChangeEvent> message)
        { 
            auto character = world.getActor<Character>(message.contents.actor);
            if(character != nullptr) {
                ItemStack stack;
                DataLoaderImpl dataLoader(registry,world.constructionMaterial);
                stack.load(message.contents.stack,dataLoader);
                if(message.contents.lose) {
                    character->take(stack);
                } else {
                    character->give(stack);
                }
            } else {
                Debug::warn("incoming message character is nullptr");
            }
        });


        client.addMessageCallback<MessageConstructionPlaceBlockEvent>([&](IncomingMessageClient<MessageConstructionPlaceBlockEvent> message)
        { 
            auto construction = world.getActor<Construction>(message.contents.actor);

            vec3 location = message.contents.position.toVec3();
            DataLoaderImpl dataLoader(registry,world.constructionMaterial);
            Block* block = dataLoader.getBlockPrototype((string)message.contents.block);
            BlockStorage storage;
            storage.load(message.contents.storage,dataLoader);

            if(construction != nullptr) {
                construction->placeBlock(location,block,storage,false);
            } else {
                Debug::warn("incoming message construction is nullptr");
            }
        });

        client.addMessageCallback<MessageConstructionBreakBlockEvent>([&](IncomingMessageClient<MessageConstructionBreakBlockEvent> message)
        { 
            auto construction = world.getActor<Construction>(message.contents.actor);

            vec3 location = message.contents.position.toVec3();
            if(construction != nullptr) {
                construction->removeBlock(location);
            } else {
                Debug::warn("incoming message construction is nullptr");
            }
        });

        server.addMessageCallback<MessageCharacterToolActionEvent>([&](IncomingMessageServer<MessageCharacterToolActionEvent> message)
        { 
            auto character = world.getActor<Character>(message.contents.actor);

            DataLoaderImpl dataLoader(registry,world.constructionMaterial);
            Item* tool = dataLoader.getItemPrototype((string)message.contents.tool);
            if(character != nullptr) {
                auto position = character->getPosition();
                auto rotation = character->getRotation();
                auto pitch = character->lookPitch;
                character->setPosition(message.contents.position.toVec3());
                character->setRotation(message.contents.rotation.toQuat());
                character->lookPitch = message.contents.lookPitch;
                character->receiveToolActionEvent(&world,tool,message.contents.actionEvent);
                character->setPosition(position);
                character->setRotation(rotation);
                character->lookPitch = pitch;
            } else {
                Debug::warn("incoming message construction is nullptr");
            }
        });

        client.addRawMessageCallback("CHAT", [&](IncomingMessageClient<string> message)
        { chatLog.push_back(message.contents); });

        world.constructionMaterial = registry.getMaterial(Loader::DEFAULT_CONSTRUCTION_MATERIAL_KEY);

        lua["world"] = &world;
    }

    // destroys player if one exists
    void destroyLocalPlayer()
    {
        auto *player = world.getActor<Actor>(playerID);
        if (player != nullptr)
        { // if theres an existing player, destroy it
            player->destroy(&world);
        }
    }

    void spawnLocalPlayer()
    {
        destroyLocalPlayer();
        auto player = world.spawn(Character::makeInstance(registry.getActor<Character>("player")));
        player->alwaysRender = false;

        player->give(ItemStack(registry.getItem("pickaxe"), 1));
        player->give(ItemStack(registry.getItem("tin_plate"), 99));
        player->give(ItemStack(registry.getItem("furnace"), 1));
        playerID = player->id;
        lua["player"] = playerID;
    }

    Actor* spawnRemotePlayer(ConnectedClient *client)
    {
        Character *character = world.spawn(Character::makeInstance(registry.getActor<Character>("player")));
        server.setClientActorID(client, character->id);
        character->networkLocal = true;
        character->give(ItemStack(registry.getItem("pickaxe"), 1));
        character->give(ItemStack(registry.getItem("tin_plate"), 99));
        character->give(ItemStack(registry.getItem("furnace"), 1));
        return character;
    }

    void serverReceiveText(string text)
    {
        chatLog.push_back(text);
        server.sendRawMessageToAllClients("CHAT", text); // just the entire chat message
    }

    void serverReceiveMessage(string name, string contents)
    {
        serverReceiveText("<" + name + "> " + contents);
    }

    void displayChatLog()
    {
        for (int i = chatLog.size() - 1; i >= 0; i--)
        {
            ImGui::Text(chatLog[i].c_str());
        }
    }

    void mainMenu()
    {
        destroyLocalPlayer();
        world.clear();
        world.onActorSpawned.unSubscribe(&worldListener);
        world.onActorDestroyed.unSubscribe(&worldListener);
        chatLog.clear();
        appState = AppState::MainMenu;
    }

    void startHost()
    {
        appState = AppState::Host;
        server.openSocket(nPort);
        spawnLocalPlayer();
        world.spawn(Actor::makeInstance(registry.getActor<Actor>("plane")));
        world.onActorSpawned.subscribe(&worldListener);
        world.onActorDestroyed.subscribe(&worldListener);
        // lua.do_file("scripts/start.lua");
    }

    void loopHost(float dt)
    {
        server.handleConnections();
        server.pollMessages();
        if (ImGui::Button("Close Server"))
        {
            server.close();
            mainMenu();
        }
        if (ImGui::InputText("Chat:", messageBuffer, IM_ARRAYSIZE(messageBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {

            serverReceiveMessage(nameBuffer, messageBuffer);
            strcpy(messageBuffer, "");
        }
        displayChatLog();
        world.frame(vulkan, dt);
        actorUpdateDt += dt;
        if (actorUpdateDt > 0.05)
        {
            sendActorUpdates();
            actorUpdateDt = 0;
        }
    }

    void startClient()
    {
        client.setName(nameBuffer);
        std::cout << "starting client" << std::endl;
        appState = AppState::Client;
        if (client.getStatus() == ClientStatus::Disconnected)
        {
            client.connectRemote(addressBuffer, nPort);
        }
        // world.spawn(Actor::makeInstance(registry.getActor<Actor>("plane")));
    }

    void loopClient(float dt)
    {
        client.handleConnections();
        switch (client.getStatus())
        {
        case ClientStatus::Disconnected:
            ImGui::Text("Disconnected");
            if (ImGui::Button("Connect"))
            {
                client.connectRemote(addressBuffer, nPort);
            }
            if (ImGui::Button("Exit"))
            {
                client.disconnect();
                mainMenu();
            }
            break;
        case ClientStatus::Connecting:
            ImGui::Text("Connecting");
            break;
        case ClientStatus::Connected:
            client.pollMessages();
            ImGui::Text("Connected!");
            if (ImGui::InputText("Chat:", messageBuffer, IM_ARRAYSIZE(messageBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
            {

                client.sendRawMessage<string>("CHAT", messageBuffer);
                strcpy(messageBuffer, "");
            }
            if (ImGui::Button("Leave"))
            {
                client.sendRawMessage<string>("LEAV", "");
                client.disconnect();
                mainMenu();
            }
            displayChatLog();
        }
        world.frame(vulkan, dt);
        auto player = world.getActor<Character>(playerID);
        if (player != nullptr)
        {
            player->stepClient(&world,dt);
        }
        actorUpdateDt += dt;
        if (actorUpdateDt > 0.05)
        {
            sendActorUpdateClient();
            actorUpdateDt = 0;
        }
    }

    void display(Camera &camera)
    {
        auto size = window->getFrameBufferSize();
        camera.setAspect(size.x, size.y);

        vulkan->render(camera);
        vulkan->clearObjects();
    }

    void clientPlayerStep(float dt)
    {
        if (playerID == Invalid_ActorID)
            return;
        auto *player = world.getActor<Character>(playerID);
        if (player != nullptr)
        {
            player->step(&world, dt);
        }
    }

    void handlePlayer(Camera &camera, Input &input,DrawContext drawContext)
    {
        if (playerID == Invalid_ActorID)
            return;
        auto *player = world.getActor<Character>(playerID);
        if (player != nullptr)
        {
            player->setCamera(camera, world.getInterpolationTime());
            if (!mouseControl && !player->inMenu)
            {
                window->setCursorMode(CursorMode::Locked);
                drawContext.disableClicks();
            }
            else
            {
                window->setCursorMode(CursorMode::Normal);
            }

            if(!mouseControl) player->processInput(input);

            if(player->widget != nullptr) {
                player->widget->draw(drawContext,*player);
            }
        }
        else
        {
            player = Invalid_ActorID;
            window->setCursorMode(CursorMode::Normal);
        }
    }

    void loop()
    {

        ImGui_ImplVulkan_NewFrame(); // these could be moved to window and vulkan specifically
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        auto &camera = world.getCamera();
        float dt = clock.reset();

        ImGui::Begin("ChatApp");
        if (!mouseControl && playerID != Invalid_ActorID)
        {
            ImGui::Text("Press F1 to regain mouse control");
        }
        switch (appState)
        {
        case AppState::MainMenu:
            if (ImGui::Button("Host"))
            {
                startHost();
            }
            if (ImGui::Button("Join"))
            {
                startClient();
            }
            ImGui::InputText("Address", addressBuffer, IM_ARRAYSIZE(addressBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            ImGui::InputText("Name", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
            break;
        case AppState::Client:
            loopClient(dt);
            break;
        case AppState::Host:
            loopHost(dt);
            break;
        }
        ImGui::End();

        DebugMenu::issuesMenu();

        auto input = window->pollInput();

        if (input.getKeyPressed(GLFW_KEY_F1))
        {
            mouseControl = !mouseControl;
        }

        DrawContext drawContext(interface,*vulkan,input);

        skybox.addRenderables(*vulkan, camera);

        camera.rotate(vec3(0, 2, 0) * dt);

        handlePlayer(camera, input,drawContext);

        display(camera);
    }

public:
    NetTestApp()
    {
        window = new Window("Chat App", 1000, 800);
        vulkan = new Vulkan("Chat App Renderer", window);
    }

    ~NetTestApp()
    {
        delete window;
        delete vulkan;
    }

    NetTestApp(NetTestApp &app) = delete;

    void run()
    {

        setup();

        while (!window->shouldClose())
        {

            loop();
        }
    }
};