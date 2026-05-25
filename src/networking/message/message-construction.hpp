#pragma once

#include <string>
#include "persistance/actor/data-construction.hpp"

#include "glm/glm.hpp"

#include "actor/construction.hpp"

using std::string;

struct MessageConstructionPlaceBlockEvent {

    static string getMessageType() { return "CPBE"; }

    cista::raw::string block;
    data_BlockStorage storage;
    data_ivec3 position;
    ActorID actor;
    
};


struct MessageConstructionBreakBlockEvent {

    static string getMessageType() { return "CBBE"; }

    data_ivec3 position;
    ActorID actor;
    
};