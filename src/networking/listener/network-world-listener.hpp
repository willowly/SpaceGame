#pragma once
#include "networking/server-socket.hpp"
#include "helper/event.hpp"
#include "engine/world.hpp"
#include "network-actor-listener.hpp"

struct NetworkWorldListener : public EventListener<World::EventActorSpawned>, public EventListener<World::EventActorDestroyed>
{
    ServerSocket* server = nullptr;
    std::map<data_ActorType,std::unique_ptr<NetworkActorListener>> actorListenerMap;

    void subscribeAll(World& world) {
        world.onActorSpawned.subscribe(this);
        world.onActorDestroyed.subscribe(this);
    }

    template<typename T>
    void registerType(data_ActorType type) {
        assert(server != nullptr);
        actorListenerMap[type] = std::move(std::make_unique<T>());
        actorListenerMap[type]->messageSender = server;
    }

    void onEvent(World::EventActorSpawned event) override
    {
        assert(server != nullptr);

        assert(event.actor != nullptr);

        
        auto type = event.actor->getActorDataType();
        if(actorListenerMap.contains(type)) {
            actorListenerMap.at(type)->subscribeAll(*event.actor);
        }

        // dont send player spawn events (they are handled manually)
        if(event.actor->getActorDataType() == data_ActorType::PLAYER) {
            return; 
        }
        auto dataOpt = event.actor->getDataEntry();
        if(dataOpt) {
            server->sendMessageToAllClients(MessageSpawnActor(dataOpt.value()));
        }

        
    }

    void onEvent(World::EventActorDestroyed event) override
    {
        assert(server != nullptr);

        assert(event.actor != nullptr);

        server->sendMessageToAllClients(MessageDestroyActor(event.actor->id));

        
    }
};