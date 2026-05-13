#pragma once

#include <chrono>
#include <stdexcept>
#include <thread>
#define TRACY_ENABLE 1
#include "tracy/Tracy.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "game-client.hpp"
#include "game-server.hpp"

#include "graphics/vulkan.hpp"
#include "engine/window.hpp"
#include "imgui/imgui.h"
#include "graphics/skybox.hpp"
//#include <tracy/Tracy.hpp>
#include "cista.h"

#include "engine/loader.hpp"

#include "persistance/data-loader-impl.hpp"

#include "networking/message/message-actor.hpp"

#include <string>

#include <iostream>

#include <format>


class NetTestApp {

    enum class AppState {
        MainMenu,
        Host,
        Client
    };

    const uint16 nPort = 27020;

    Window* window;

    Vulkan* vulkan;

    Loader loader;

    sol::state lua;

    Registry registry;
    Clock clock;

    GameServer server;
    GameClient client;

    Skybox skybox;

    World world;

    float actorUpdateDt = {};

    bool mouseControl = false;

    ActorID playerID = Invalid_ActorID;

    AppState appState = AppState::MainMenu;

    char addressBuffer[1024] = "127.0.0.1";
    char messageBuffer[1024] = "";
    char nameBuffer[1024] = "user";


    std::vector<string> chatLog;

    void sendActorToClient(Actor* actor,const ConnectedClient client) {
        actor->createSaveBuffer();
        data_ActorEntry data_entry;
        data_ActorType type = actor->getActorDataType();
        if(type == data_ActorType::DONT_SAVE)
            return;

        data_entry.type = type;
        auto buf = actor->createSaveBuffer();
        data_entry.name = actor->name;
        data_entry.data.reserve(buf.size());
        for (size_t i = 0; i < buf.size(); i++)
        {
            data_entry.data.push_back(buf[i]);
        }

        server.sendMessageToClient(client,MessageSpawnActor(data_entry));
    }

    void sendActorUpdates() {
        for(auto* actor : world.getActors()) {
            if(actor->networkLocal) {
                auto message = MessageUpdateActorTransform(actor->id);
                message.newPosition.set(actor->getPosition());
                message.newRotation.set(actor->getRotation());
                server.sendMessageToAllClients(message);
            }
        }
    }

    void sendActorUpdateClient() {
        auto actor = world.getActor<Actor>(playerID);
        if(actor != nullptr) {
            auto message = MessageUpdateActorTransform(actor->id);
            message.newPosition.set(actor->getPosition());
            message.newRotation.set(actor->getRotation());
            client.sendMessage(message);
        }
    }

    void receiveActorUpdate(MessageUpdateActorTransform contents) {
        auto actor = world.getActor<Actor>(contents.id);

        if(actor != nullptr) {
            actor->setPosition(contents.newPosition.toVec3());
            actor->setRotation(contents.newRotation.toQuat());
            actor->updateLastTransform();
        } else {
            Debug::warn("couldn't follow actor update... ");
        }
    }

    void setup() {

        lua.open_libraries(sol::lib::base,sol::lib::package);
        API::loadAPIAll(lua);

        vulkan->waitIdle();

        loader.loadAll(registry,lua,vulkan);

        SkyboxMaterialData skyboxMaterial;
        skyboxMaterial.top = registry.getTexture("space_up");
        skyboxMaterial.bottom = registry.getTexture("space_dn");
        skyboxMaterial.left = registry.getTexture("space_lf");
        skyboxMaterial.right = registry.getTexture("space_rt");
        skyboxMaterial.front = registry.getTexture("space_ft");
        skyboxMaterial.back = registry.getTexture("space_bk");

        skybox.loadResources(*vulkan,skyboxMaterial);

        server.addRawMessageCallback("CHAT", [&](IncomingMessageServer<string> message) {
            serverReceiveMessage(message.client->getName(),message.contents);
        });

        server.addRawMessageCallback("JOIN", [&](IncomingMessageServer<string> message) {
            server.setClientName(message.client,message.contents);
            serverReceiveText(message.contents + " has joined!");
            for(auto* actor : world.getActors()) {
                sendActorToClient(actor,*message.client);
            }
            auto newPlayer = spawnRemotePlayer(message.client);
            sendActorToClient(newPlayer,*message.client);
            server.sendMessageToClient(*message.client,MessageSetPlayerControl(newPlayer->id)); //tell the player who they control
        });

        server.addRawMessageCallback("LEAV", [&](IncomingMessageServer<string> message) {
            serverReceiveText(message.client->getName() + " has left");
            if(message.client->actorID != Invalid_ActorID) {
                world.destroyActor(message.client->actorID);
            }
        });

        client.addMessageCallback<MessageSpawnActor>([&](IncomingMessageClient<MessageSpawnActor> message) {
            
            DataLoaderImpl dataLoader(registry,Material::none);
            auto actor = world.loadActor(message.contents.actorEntry,dataLoader);
            if(actor != nullptr) {
                actor->networkLocal = false;
            }
        });

        client.addMessageCallback<MessageSetPlayerControl>([&](IncomingMessageClient<MessageSetPlayerControl> message) {
            
            playerID = message.contents.id;
            
        });

        client.addMessageCallback<MessageUpdateActorTransform>([&](IncomingMessageClient<MessageUpdateActorTransform> message) {
            

            receiveActorUpdate(message.contents);

            
        });

        server.addMessageCallback<MessageUpdateActorTransform>([&](IncomingMessageServer<MessageUpdateActorTransform> message) {

            receiveActorUpdate(message.contents);

        });

        client.addRawMessageCallback("CHAT", [&](IncomingMessageClient<string> message) {
            chatLog.push_back(message.contents);
        });

        lua["world"] = &world;

    }

    // destroys player if one exists
    void destroyLocalPlayer() {
        auto* player = world.getActor<Actor>(playerID);
        if(player != nullptr) { //if theres an existing player, destroy it
            player->destroy(&world);
        }
    }

    void spawnLocalPlayer() {
        destroyLocalPlayer();
        auto player = world.spawn(Character::makeInstance(registry.getActor<Character>("player")));
        player->alwaysRender = false;
        playerID = player->id;
        lua["player"] = playerID;
    }

    Actor* spawnRemotePlayer(ConnectedClient* client) {
        Actor* actor = world.spawn(Character::makeInstance(registry.getActor<Character>("player")));
        server.setClientActorID(client,actor->id);
        actor->networkLocal = false;
        return actor;
    }

    void serverReceiveText(string text) {
        chatLog.push_back(text);
        server.sendRawMessageToAllClients("CHAT",text); // just the entire chat message
    }

    void serverReceiveMessage(string name,string contents) {
        serverReceiveText("<" + name + "> " + contents);
    }

    void displayChatLog() {
        for (int i = chatLog.size() - 1; i >= 0; i--)
        {
            ImGui::Text(chatLog[i].c_str());
        }
    }

    void mainMenu() {
        destroyLocalPlayer();
        world.clear();
        chatLog.clear();
        appState = AppState::MainMenu;
    }

    void startHost() {
        appState = AppState::Host;
        server.openSocket(nPort);
        spawnLocalPlayer();
        world.spawn(Actor::makeInstance(registry.getActor<Actor>("plane")));
        //lua.do_file("scripts/start.lua");
    }

    void loopHost(float dt) {
        server.handleConnections();
        server.pollMessages();
        if(ImGui::Button("Close Server")) {
            server.close();
            mainMenu();
        }
        if(ImGui::InputText("Chat:",messageBuffer,IM_ARRAYSIZE(messageBuffer),ImGuiInputTextFlags_EnterReturnsTrue)) {

            serverReceiveMessage(nameBuffer,messageBuffer);
            strcpy(messageBuffer,"");
        }
        displayChatLog();
        world.frame(vulkan,dt);
        actorUpdateDt += dt;
        if(actorUpdateDt > 0.05) {
            sendActorUpdates();
            actorUpdateDt = 0;
        }
    }

    void startClient() {
        client.setName(nameBuffer);
        std::cout << "starting client" << std::endl;
        appState = AppState::Client;
        if(client.getStatus() == ClientStatus::Disconnected) {
            client.connectRemote(addressBuffer,nPort);
        }
        //world.spawn(Actor::makeInstance(registry.getActor<Actor>("plane")));
    }

    void loopClient(float dt) {
        client.handleConnections();
        switch(client.getStatus()) {
            case ClientStatus::Disconnected:
                ImGui::Text("Disconnected");
                if(ImGui::Button("Connect")) {
                    client.connectRemote(addressBuffer,nPort);
                }
                if(ImGui::Button("Exit")) {
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
                if(ImGui::InputText("Chat:",messageBuffer,IM_ARRAYSIZE(messageBuffer),ImGuiInputTextFlags_EnterReturnsTrue)) {

                    client.sendRawMessage<string>("CHAT",messageBuffer);
                    strcpy(messageBuffer,"");
                }
                if(ImGui::Button("Leave")) {
                    client.sendRawMessage<string>("LEAV","");
                    client.disconnect();
                    mainMenu();
                }
                displayChatLog();
        }
        world.frame(vulkan,dt);
        actorUpdateDt += dt;
        if(actorUpdateDt > 0.05) {
            sendActorUpdateClient();
            actorUpdateDt = 0;
        }
    }

    void display(Camera& camera) {
        auto size = window->getFrameBufferSize();
        camera.setAspect(size.x,size.y);

        vulkan->render(camera);
        vulkan->clearObjects();
    }

    void clientPlayerStep(float dt) {
        if(playerID == Invalid_ActorID) return;
        auto* player = world.getActor<Character>(playerID);
        if(player != nullptr) {
            player->step(&world,dt);
        }
    }


    void handlePlayer(Camera& camera,Input& input) {
        if(playerID == Invalid_ActorID) return;
        auto* player = world.getActor<Character>(playerID);
        if(player != nullptr) {
            player->networkLocal = true;
            player->alwaysRender = false;
            player->setCamera(camera,world.getInterpolationTime());
            if(!mouseControl) {
                player->processInput(input);
                window->setCursorMode(CursorMode::Locked);
            } else {
                window->setCursorMode(CursorMode::Normal);
            }
        } else {
            player = Invalid_ActorID;
            window->setCursorMode(CursorMode::Normal);
        }
    }

    void loop() {

        ImGui_ImplVulkan_NewFrame(); // these could be moved to window and vulkan specifically
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        auto& camera = world.getCamera();
        float dt = clock.reset();

        ImGui::Begin("ChatApp");
            if(!mouseControl && playerID != Invalid_ActorID) {
                ImGui::Text("Press F1 to regain mouse control");
            }
            switch(appState) {
                case AppState::MainMenu:
                    if(ImGui::Button("Host")) {
                        startHost();
                    }
                    if(ImGui::Button("Join")) {
                        startClient();
                    }
                    ImGui::InputText("Address",addressBuffer,IM_ARRAYSIZE(addressBuffer),ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::InputText("Name",nameBuffer,IM_ARRAYSIZE(nameBuffer),ImGuiInputTextFlags_EnterReturnsTrue);
                    break;
                case AppState::Client:
                    loopClient(dt);
                    break;
                case AppState::Host:
                    loopHost(dt);
                    break;
            }
        ImGui::End();
        
        auto input = window->pollInput();

        if(input.getKeyPressed(GLFW_KEY_F1)) {
            mouseControl = !mouseControl;
        }

        skybox.addRenderables(*vulkan,camera);


        camera.rotate(vec3(0,2,0) * dt);

        handlePlayer(camera,input);

        display(camera);

    }

    public:
        NetTestApp() {
            window = new Window("Chat App",1000,800);
            vulkan = new Vulkan("Chat App Renderer",window);
        }

        ~NetTestApp() {
            delete window;
            delete vulkan;
        }

        NetTestApp(NetTestApp& app) = delete;

        void run() {
            
            setup();

            while(!window->shouldClose()) {
                
                loop();
            }
        }
};