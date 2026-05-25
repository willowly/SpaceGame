#pragma once

#include <string>
#include "persistance/actor/data-actor.hpp"

#include "glm/glm.hpp"

#include "actor/actor.hpp"

using std::string;

struct MessageCharacterUpdateItemEvent {

    static string getMessageType() { return "CUIE"; }

    //MessageCharacterUpdateItemEvent(string itemName) : itemName(itemName) {}

    cista::raw::string itemName;
    ActorID actor;
    int slot;
    
};

struct MessageCharacterInventoryChangeEvent {

    static string getMessageType() { return "CICE"; }

    //MessageCharacterUpdateItemEvent(string itemName) : itemName(itemName) {}

    data_ItemStack stack;
    ActorID actor;
    bool lose = false; //if the player should gain/lose. Lose is false
    
};


struct MessageCharacterDropItemEvent {
    static string getMessageType() { return "CDIE"; }
    data_ItemStack stack;
    ActorID actor;
    data_vec3 position;
};

struct MessageCharacterToolActionEvent {
    static string getMessageType() { return "CTAE"; }
    ActorID actor;
    cista::raw::string tool;
    int actionEvent;
    data_vec3 position;
    data_quat rotation;
    float lookPitch;
};