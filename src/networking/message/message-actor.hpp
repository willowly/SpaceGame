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

// server telling the client "hey you control this one"
struct MessageSetPlayerControl {

    static string getMessageType() { return "SPLC"; }

    ActorID id = Invalid_ActorID;
    
};

struct MessageUpdateActorTransform {

    static string getMessageType() { return "UACT"; }

    ActorID id = Invalid_ActorID;
    data_vec3 newPosition;
    data_quat newRotation;
    
};