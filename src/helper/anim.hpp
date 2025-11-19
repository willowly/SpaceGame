#define PI 3.14159265

#include <math.h>

namespace Anim {


    float easeInSine(float t) {
        return 1 - cos((t * PI) / 2);
    }

    float easeOutSine(float t) {
        return sin((t * PI) / 2);
    }

    float easeOutCubic(float t) {
        return 1 - pow(1 - t, 3);
    }

    float easeInCube(float t) {
        return t * t * t;
    }

    float easeInOutCubic(float t) {
        return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2;
    }
}