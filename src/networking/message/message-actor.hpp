#pragma once

#include <string>
#include "persistance/actor/data-actor.hpp"

#include "glm/glm.hpp"

#include "actor/actor.hpp"

using std::string;

struct MessageSpawnActor {

    static string getMessageType() { return "SPWN"; }

    data_ActorEntry actorEntry;
    bool localPlayer;
    
};

struct MessageDestroyActor {

    static string getMessageType() { return "DSRY"; }

    ActorID id = Invalid_ActorID;
    
};

struct MessageUpdateActorTransform {

    static string getMessageType() { return "UACT"; }

    ActorID id = Invalid_ActorID;
    data_vec3 newPosition;
    data_quat newRotation;
    
};