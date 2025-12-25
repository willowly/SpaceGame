#pragma once


#include <graphics/mesh.hpp>
#include <item/item-stack.hpp>



class Construction;
struct BlockState;
class Character;


class Block {

    public:
        Mesh<Vertex>* model;
        TextureID texture;
        bool canRide;
        Item* drop;

        
        Block(Mesh<Vertex>* model,TextureID texture) : model(model), texture(texture) {
            
        }
        Block() : Block(nullptr,0) {}
        
        virtual ~Block() = default;

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {

        }

        // needs to use construction.addStepCallback() to make work
        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockState& state,float dt) {

        }

        virtual optional<ItemStack> getDrop(BlockState& state) {
            if(drop == nullptr) {
                return std::nullopt;
            } else {
                return ItemStack(drop,1);
            }
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {}

};