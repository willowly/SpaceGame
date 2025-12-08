#pragma once

#include "glm/glm.hpp"

using glm::ivec3;

struct LocationKey {
    int x = 0;
    int y = 0;
    int z = 0;
    LocationKey(int x,int y,int z) : x(x), y(y), z(z) {

    }
    LocationKey(ivec3 i) : x(i.x), y(i.y), z(i.z) {
        
    }
    ivec3 asVec3() const {
        return ivec3(x,y,z);
    }

    constexpr bool operator==(const LocationKey&) const = default;

    constexpr auto operator<=>(const LocationKey&) const = default;

};