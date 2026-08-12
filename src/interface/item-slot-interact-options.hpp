#pragma once 


struct ItemSlotInteractOptions {

    bool allowInsert = true;
    bool allowRemove = true;
    float spaceLeft = std::numeric_limits<float>().max();
};
