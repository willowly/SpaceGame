#pragma once
#include "networking/message-sender.hpp"
#include "actor/character.hpp"
#include "network-actor-listener.hpp"
#include "networking/message/message-character.hpp"


struct NetworkCharacterListener : 
EventListener<Character::EventHeldItemChanged>, 
EventListener<Character::EventInventoryChanged>,  
EventListener<Character::EventItemDropInput>, 
EventListener<Character::EventToolAction>, 
NetworkActorListener
    {


        void subscribeAll(Actor& actor) override {
            auto& character = dynamic_cast<Character&>(actor);
            character.onInventoryChanged.subscribe(this);
            character.onHeldItemChanged.subscribe(this);
        }

        void subscribeClient(Actor& actor) {
            auto& character = dynamic_cast<Character&>(actor);
            character.onHeldItemChanged.subscribe(this);

            character.onItemDropInput.subscribe(this); // player input event
            character.onToolAction.subscribe(this); // player input event
        }


        void onEvent(Character::EventInventoryChanged event) override
        {
            assert(messageSender != nullptr);
            assert(event.character != nullptr);

            MessageCharacterInventoryChangeEvent message(event.stack.save(),event.character->id,event.lose);
            messageSender->sendMessage(message);
            
        }

        void onEvent(Character::EventHeldItemChanged event) override
        {
            assert(messageSender != nullptr);
            assert(event.character != nullptr);
            string name;
            if(!event.stack.isEmpty()) {
                name = event.stack.item->name;
            } else {
                name = "";
            }
            MessageCharacterUpdateItemEvent message(name,event.character->id,event.slot);
            messageSender->sendMessage(message);
        }

        void onEvent(Character::EventItemDropInput event) override 
        {
            assert(messageSender != nullptr);
            assert(event.character != nullptr);

            MessageCharacterDropItemEvent message;
            message.actor = event.character->id;
            message.stack = event.character->getHeldItemStack().save();
            message.position.set(event.character->getItemDropPosition());
            messageSender->sendMessage(message);
        }

        void onEvent(Character::EventToolAction event) override 
        {
            assert(messageSender != nullptr);
            assert(event.character != nullptr);

            MessageCharacterToolActionEvent message;
            message.actor = event.character->id;
            message.tool = event.tool->name;
            message.actionEvent = event.actionEvent;
            message.rotation.set(event.character->getRotation());
            message.position.set(event.character->getPosition());
            message.lookPitch = event.character->lookPitch;
            messageSender->sendMessage(message);
        }
    };