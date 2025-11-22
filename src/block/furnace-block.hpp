#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"

#include "actor/furnace-block-actor.hpp"


class FurnaceBlock : public Block {
    public:

    
        FurnaceBlock(Mesh<Vertex>* model,Material material) : Block(model,material) {
            
        }
        FurnaceBlock() : FurnaceBlock(nullptr,Material::none) {}

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            construction->spawnBlockActor<FurnaceBlockActor>(position);
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            std::cout << "wow" << std::endl;
            character.setActionInventory();
        }
};