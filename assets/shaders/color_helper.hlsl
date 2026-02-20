#pragma once

vec3 toLinear(vec3 inColor) {
    float gamma = 2.2;
    return pow(inColor, vec3(gamma));
}

vec4 toLinear(vec4 inColor) {
    float gamma = 2.2;
    return pow(inColor, vec4(gamma));
}