#pragma once


#include <graphics/model.hpp>




class Construction;
struct BlockState;
class Character;


class Block {

    public:
        Model* model;
        Material material;
        bool canRide;

        
        Block(Model* model,Material material) : model(model), material(material) {
            
        }
        Block() : Block(nullptr,Material()) {}
        
        virtual ~Block() = default;

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {

        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {}

};