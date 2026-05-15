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
    
};
