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

    BlockFacing facing;

    BlockState(BlockFacing facing = BlockFacing::FORWARD) : facing(facing) {}

};