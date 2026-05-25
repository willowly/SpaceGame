#pragma once
#include "actor/actor.hpp"
#include "networking/message-sender.hpp"



struct NetworkActorListener {

    IMessageSender* messageSender = nullptr;

    virtual void subscribeAll(Actor& actor) = 0;

    virtual ~NetworkActorListener() {}
};