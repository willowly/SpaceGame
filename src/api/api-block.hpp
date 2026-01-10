#pragma once

#include <sol/sol.hpp>
#include "api-general.hpp"
#include "block/block.hpp"

namespace API {

    void loadAPIBlock(sol::state& lua) {
        
        sol::usertype<Block> block = lua.new_usertype<Block>("block",sol::no_constructor);
        
        block["drop"] = &Block::drop;
        // block["texture"] = &Block::texture;
        // block["model"] = &Block::mesh;

    }
}