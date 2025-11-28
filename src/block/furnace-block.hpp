#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "interface/block/block-menu-object.hpp"
#include "interface/block/block-widget.hpp"
#include "item/recipe.hpp"



class FurnaceBlock : public Block {
    public:



        float fuelMax = 10;
        float craftSpeed = 1;

        //floats
        const int FUEL_VAR = 0;
        const int TIMER_VAR = 1;

        //stacks
        const int INPUTSTACK_VAR = 0;
        const int OUTPUTSTACK_VAR = 1;

        //pointers
        const int CURRENTRECIPE_VAR = 0;

        BlockWidget<FurnaceBlock>* widget;
        std::vector<Recipe*> recipes;
    
        FurnaceBlock(Mesh<Vertex>* model,Material material) : Block(model,material) {
            
        }
        FurnaceBlock() : FurnaceBlock(nullptr,Material::none) {}

        virtual void onPlace(Construction* construction,ivec3 position,BlockState& state) {
            auto storage = construction->addStorage(position);
            construction->addStepCallback(position);
            if(storage == nullptr) {
                throw std::runtime_error("Storage was null for furnace!");
            } 
            storage->setFloat(FUEL_VAR,fuelMax);
            storage->setStack(INPUTSTACK_VAR,ItemStack(nullptr,0));
            storage->clearStack(INPUTSTACK_VAR);
            storage->setPointer<Recipe>(CURRENTRECIPE_VAR,nullptr);
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            if(widget == nullptr) {
                Debug::warn("furnace menu null");
                return;
            }
            auto menuObj = std::make_unique<BlockMenuObject<FurnaceBlock>>(construction,position,*widget);
            character.openMenu(std::move(menuObj));
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockState& state,float dt) {
            auto storage = construction->getStorage(position);
            if(storage == nullptr) {
                throw std::runtime_error("Storage was null for furnace!");
            }

            float fuel = storage->getFloat(FUEL_VAR);
            float craftTimer = storage->getFloat(TIMER_VAR);
            Recipe* recipe = storage->getPointer<Recipe>(CURRENTRECIPE_VAR);
            ItemStack inputStackOpt = storage->getStack(INPUTSTACK_VAR);
            ItemStack outputStackOpt = storage->getStack(OUTPUTSTACK_VAR);
            if(recipe != nullptr) {
                craftStep(craftTimer,*recipe,inputStackOpt,outputStackOpt,dt);
            }
            storage->setFloat(FUEL_VAR,fuel);
            storage->setFloat(TIMER_VAR,craftTimer);
            storage->setStack(INPUTSTACK_VAR,inputStackOpt);
            storage->setStack(OUTPUTSTACK_VAR,outputStackOpt);
        }

        void craftStep(float& craftTimer,Recipe& recipe,ItemStack& inputStack,ItemStack& outputStack,float dt) {
            if(recipe.ingredients.size() == 1) {
                std::cout << "furnace crafting " << recipe.result.item->name << " " << craftTimer << std::endl;
                craftTimer += dt * craftSpeed;
                if(craftTimer > recipe.time) {
                    auto ingredient = recipe.ingredients[0];
                    if(inputStack.has(ingredient)) {
                        inputStack.amount -= ingredient.amount;
                    }
                    craftTimer = 0;
                }
            } else {
                Debug::warn("furnace recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
                return;
            }
        }

        virtual void tryStartCraft(Recipe& recipe,Character& character,GenericStorage& storage,BlockState& state) {
            if(!character.inventory.hasIngredients(recipe)) {
                return;
            }
            if(recipe.ingredients.size() != 1) {
                Debug::warn("furnace recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
                return;
            }
            character.inventory.take(recipe.ingredients[0]); //should have them so we dont need to check
            storage.setStack(INPUTSTACK_VAR,recipe.ingredients[0]);
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,&recipe);
            storage.setFloat(TIMER_VAR,0);

        }
};