#define GLM_ENABLE_EXPERIMENTAL
#include "construction.hpp"

bool operator< (const Construction::Location& a,const Construction::Location& b) {
    if(a.x == b.x) {
        if(a.y == b.y) {
            return a.z < b.z;
        }
        return a.y < b.y;
    }
    return a.x < b.x;
}

const Color Construction::debugGroupColors[] = {
    Color::green,
    Color::blue,
    Color::red,
    Color::magenta,
    Color::cyan,
    Color::yellow
};