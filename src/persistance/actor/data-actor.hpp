#pragma once
#include "persistance/data-generic.hpp"
#include "cista.h"

#include <array>
#include <memory>
#include <functional>

class Actor;
class Registry;

enum data_ActorType {
    DUMMY,
    PLAYER,
    PHYSICS,
    CONSTRUCTION,
    DONT_SAVE,
    ACTOR_TYPES_COUNT
};


struct data_ActorEntry {
    data_ActorType type;
    cista::raw::string name;
    cista::raw::vector<std::uint8_t> data;
};

struct data_Actor {
    data_vec3 position;
    data_quat rotation;
};