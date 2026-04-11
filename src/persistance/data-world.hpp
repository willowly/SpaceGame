#pragma once
#include "cista.h"

#include "actor/data-actor.hpp"



struct data_World {
    cista::raw::vector<data_ActorEntry> actors;
};