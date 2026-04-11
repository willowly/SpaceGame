#pragma once

#include "persistance/data-generic.hpp"
#include "cista.h"

struct data_GenericStorage {
    cista::raw::vector<int> ints;
    cista::raw::vector<float> floats;
    cista::raw::vector<cista::raw::string> strings;
};