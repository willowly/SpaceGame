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

        Mesh<Vertex>* mesh;
        TextureID texture;
    
        FurnaceBlock() : Block() {
        }

        virtual BlockState onPlace(Construction* construction,ivec3 position,BlockFacing facing) {
            auto storage = construction->addStorage(position);
            construction->addStepCallback(position);
            if(storage == nullptr) {
                throw std::runtime_error("Storage was null for furnace!");
            } 
            storage->setFloat(FUEL_VAR,fuelMax);
            storage->setStack(INPUTSTACK_VAR,ItemStack(nullptr,0));
            storage->clearStack(INPUTSTACK_VAR);
            storage->setPointer<Recipe>(CURRENTRECIPE_VAR,nullptr);
            return BlockState::encode(facing);
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockState& state,Character& character) {
            if(widget == nullptr) {
                Debug::warn("furnace menu null");
                return;
            }
            auto menuObj = std::make_unique<BlockMenuObject<FurnaceBlock>>(construction,position,*widget);
            character.openMenu(std::move(menuObj));
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockState& state) {
            BlockHelper::addMesh(meshData,position,state.asFacing(),mesh,texture);
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockState& state,float dt) {
            auto storage = construction->getStorage(position);
            if(storage == nullptr) {
                throw std::runtime_error("Storage was null for furnace!");
            }

            float fuel = storage->getFloat(FUEL_VAR);
            float craftTimer = storage->getFloat(TIMER_VAR);
            Recipe* currentRecipe = storage->getPointer<Recipe>(CURRENTRECIPE_VAR);
            ItemStack inputStackOpt = storage->getStack(INPUTSTACK_VAR);
            ItemStack outputStackOpt = storage->getStack(OUTPUTSTACK_VAR);
            if(currentRecipe != nullptr) {
                craftStep(craftTimer,currentRecipe,inputStackOpt,outputStackOpt,dt);
            }
            storage->setFloat(FUEL_VAR,fuel);
            storage->setFloat(TIMER_VAR,craftTimer);
            storage->setStack(INPUTSTACK_VAR,inputStackOpt);
            storage->setStack(OUTPUTSTACK_VAR,outputStackOpt);
        }

        void trySetMatchingRecipe(Recipe*& currentRecipe,ItemStack& inputStack) {
            for(auto recipe : recipes) {
                if(inputStack.has(recipe->ingredients[0])) { //for just one ingredient
                    currentRecipe = recipe;
                    return;
                }
            }
            currentRecipe = nullptr;
        }

        void craftStep(float& craftTimer,Recipe*& currentRecipe,ItemStack& inputStack,ItemStack& outputStack,float dt) {
            if(currentRecipe->ingredients.size() == 1) {
                auto ingredient = currentRecipe->ingredients[0];
                if(!inputStack.has(ingredient)) {
                    craftTimer = 0;
                    trySetMatchingRecipe(currentRecipe,inputStack);
                    return;
                }
                craftTimer += dt * craftSpeed;
                if(craftTimer > currentRecipe->time) {
                    
                    if(inputStack.has(ingredient)) {
                        if(outputStack.tryInsert(currentRecipe->result)) {
                            inputStack.amount -= ingredient.amount;
                            craftTimer = 0;
                        }
                    }
                }
            } else {
                Debug::warn("furnace recipe has " + std::to_string(currentRecipe->ingredients.size()) + " ingredients (must be 1)");
                return;
            }
        }

        virtual void tryStartCraft(Recipe& recipe,Character& character,BlockStorage& storage,BlockState& state) {
            if(!character.inventory.hasIngredients(recipe)) {
                return;
            }
            if(recipe.ingredients.size() != 1) {
                Debug::warn("furnace recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
                return;
            }
             //should have them so we dont need to check

            auto currentRecipe = storage.getPointer<Recipe>(CURRENTRECIPE_VAR);
            if(currentRecipe != &recipe) {
                storage.setFloat(TIMER_VAR,0);
                currentRecipe = &recipe;
            }

            auto inputStack = storage.getStack(INPUTSTACK_VAR);
            if(inputStack.tryInsert(recipe.ingredients[0])) {
                character.inventory.take(recipe.ingredients[0]);
            } else {
                // try to take the item out
                if(!inputStack.isEmpty()) {
                    character.inventory.give(inputStack);
                    inputStack.clear();
                    // try again
                    if(inputStack.tryInsert(recipe.ingredients[0])) {
                        character.inventory.take(recipe.ingredients[0]);
                    }
                }
            }
            storage.setStack(INPUTSTACK_VAR,inputStack);
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,currentRecipe);
            

        }
};