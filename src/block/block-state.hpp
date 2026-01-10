#pragma once

enum BlockFacing {
    FORWARD,
    BACKWARD,
    UP,
    DOWN,
    RIGHT,
    LEFT,
};


struct BlockState {

    uint64_t value;

    BlockState(uint64_t value) : value(value) {}

    template <typename T>
    static BlockState encode(T obj) {
        static_assert(sizeof(T) <= 8,"object is too large to encode");
        uint64_t value = 0;
        memcpy(&value,&obj,sizeof(obj));
        return BlockState(value);
    }

    template <typename T>
    T decode() {
        static_assert(sizeof(T) <= 8,"object is too large to decode");
        T obj;
        memcpy(&obj,&value,sizeof(obj));
        return obj;
    }

    BlockFacing asFacing() {
        return decode<BlockFacing>();
    }

    static BlockState none;

};

BlockState BlockState::none = BlockState(0);