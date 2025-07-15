#include <cstdlib>

namespace Random {

    float random(float min,float max) {
        float delta = max - min;

        return min + (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX))) * delta;
    }
}