#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "interface/i-has-menu.hpp"
#include "interface/block/block-widget.hpp"
#include "item/recipe.hpp"



class FurnaceBlock : public Block, public IHasMenu {
    public:



        float fuelMax = 10;

        const int FUELVAR = 0;

        BlockWidget<FurnaceBlock>* widget;
        std::vector<Recipe*> recipes;
    
        FurnaceBlock(Mesh<Vertex>* model,Material material) : Block(model,material) {
            
        }
        FurnaceBlock() : FurnaceBlock(nullptr,Material::none) {}

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            auto storage = construction->addStorage(position);
            construction->addStepCallback(position);
            if(storage != nullptr) {
                storage->setFloat(FUELVAR,fuelMax);
            } else {
                throw std::runtime_error("Storage was null for furnace!");
            }
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            character.openMenu(static_cast<IHasMenu*>(this));
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockState& state,float dt) {
            auto storage = construction->getStorage(position);
            if(storage == nullptr) {
                throw std::runtime_error("Storage was null for furnace!");
            }

            float fuel = storage->getFloat(FUELVAR);
            if(fuel > 0) {
                fuel -= dt;
                std::cout << fuel << std::endl;
            }
            storage->setFloat(FUELVAR,fuel);

            
            

        }

        void drawMenu(DrawContext context,Character& user) {
            if(widget != nullptr) {
                widget->draw(context,user,*this);
            } else {
                Debug::warn("widget for furnace block is not loaded");
            }
        }
};