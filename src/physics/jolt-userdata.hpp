#pragma once

#include "actor/actor.hpp"

struct ActorUserData {
    Actor* actor;
    unsigned int component = 0;

    ActorUserData() {}

    ActorUserData(Actor* actor) : actor(actor) { }

    ActorUserData(Actor* actor,int component) : actor(actor), component(component) {}

    static uint64_t encode(ActorUserData* userData) {
        
        return reinterpret_cast<uint64_t>(userData);
    }

    static ActorUserData* decode(uint64_t value) {
        
        return reinterpret_cast<ActorUserData*>(value);
    }


};