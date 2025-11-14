#pragma once


#include <graphics/model.hpp>




class Construction;
struct BlockState;
class Character;


class Block {

    public:
        Mesh* model;
        Material material;
        bool canRide;

        
        Block(Mesh* model,Material material) : model(model), material(material) {
            
        }
        Block() : Block(nullptr,Material::none) {}
        
        virtual ~Block() = default;

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockState& state) {

        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {}

};