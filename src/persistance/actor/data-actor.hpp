#pragma once
#include "persistance/data-generic.hpp"
#include "cista.h"

#include <array>
#include <memory>
#include <functional>

class Actor;
class Registry;


struct data_ActorEntry {
    cista::raw::string type;
    cista::raw::string name;
    cista::raw::vector<std::uint8_t> data;
};

struct data_Actor {
    unsigned int id;
    data_vec3 position;
    data_quat rotation;
};