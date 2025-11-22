#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"


class FurnaceBlock : public Block {
    public:

    
        FurnaceBlock(Mesh<Vertex>* model,Material material) : Block(model,material) {
            
        }
        FurnaceBlock() : FurnaceBlock(nullptr,Material::none) {}

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            std::cout << "wow" << std::endl;
            character.setActionInventory();
        }
};