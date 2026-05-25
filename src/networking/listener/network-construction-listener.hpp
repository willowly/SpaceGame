#pragma once
#include "networking/message-sender.hpp"
#include "actor/construction.hpp"
#include "network-actor-listener.hpp"
#include "networking/message/message-construction.hpp"


struct NetworkConstructionListener : 
    EventListener<Construction::EventBlockPlaced>, 
    EventListener<Construction::EventBlockBroken>,
    NetworkActorListener
    {


        void subscribeAll(Actor& actor) override {
            auto& construction = dynamic_cast<Construction&>(actor);
            construction.onBlockPlaced.subscribe(this);
            construction.onBlockBroken.subscribe(this);
        }


        void onEvent(Construction::EventBlockPlaced event) override
        {
            assert(messageSender != nullptr);
            assert(event.construction != nullptr);

            std::cout << " block placed at " << StringHelper::toString(event.position) << std::endl;

            MessageConstructionPlaceBlockEvent message;
            message.actor = event.construction->id;
            message.block = event.blockEntry.block->name;
            message.storage = event.blockEntry.storage.save();
            message.position.set(event.position);

            messageSender->sendMessage(message);
            
        }

        void onEvent(Construction::EventBlockBroken event) override
        {
            assert(messageSender != nullptr);
            assert(event.construction != nullptr);
            
            std::cout << " block broken at " << StringHelper::toString(event.position) << std::endl;

            MessageConstructionBreakBlockEvent message;
            message.actor = event.construction->id;
            message.position.set(event.position);

            messageSender->sendMessage(message);
        }
    };