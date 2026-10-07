#pragma once


int pattern[] = 
{
    0,32,8,40,2,34,10,42,
    48,16,56,24,50,18,58,
    12,44,4,36,14,46,6,38,
    60,28,52,20,62,30,54,22,
    3,35,11,43,1,33,9,41,
    51,19,59,27,49,17,57,25,
    15,47,7,39,13,45,5,37,
    63,31,55,23,61,29,53,21
};


bool distanceDither(vec3 viewPosition,float minDist,float maxDist) {

    ivec2 screenPos = ivec2(floor(gl_FragCoord));
        
    int patternSize = 8;
    float bias = pattern[screenPos.x % patternSize + (screenPos.y % patternSize) * patternSize];
    bias /= (patternSize*patternSize);

    float distanceNormalized = (-viewPosition.z - minDist)/(maxDist-minDist);

    return bias < distanceNormalized;

}