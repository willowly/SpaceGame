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

        //ints
        static const int FACING_VAR = 0;

        //floats
        static const int FUEL_VAR = 0;
        static const int TIMER_VAR = 1;

        //stacks
        static const int INPUTSTACK_VAR = 0;
        static const int OUTPUTSTACK_VAR = 1;

        //pointers
        static const int CURRENTRECIPE_VAR = 0;

        BlockWidget<FurnaceBlock>* widget;
        std::vector<Recipe*> recipes;

        Mesh<Vertex>* mesh;
        TextureID texture;
    
        FurnaceBlock() : Block() {
        }

        StorageType getStorageType() override {
            return StorageType::Unique;
        }

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            construction->addStepCallback(position);
            storage.setFacing(FACING_VAR,facing);
            storage.setFloat(FUEL_VAR,fuelMax);
            storage.setStack(INPUTSTACK_VAR,ItemStack(nullptr,0));
            storage.clearStack(INPUTSTACK_VAR);
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,nullptr);
            return storage;
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            if(widget == nullptr) {
                Debug::warn("furnace menu null");
                return;
            }
            auto menuObj = std::make_unique<BlockMenuObject<FurnaceBlock>>(construction,position,*widget);
            character.openMenu(std::move(menuObj));
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            quat rotation = BlockHelper::getRotationFromFacing(storage.getFacing(FACING_VAR)) * glm::quat(glm::radians(vec3(90.0f,0.0f,0.0f)));
            BlockHelper::addMesh(meshData,position,rotation,mesh->meshData,texture);
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) {

            float fuel = storage.getFloat(FUEL_VAR);
            float craftTimer = storage.getFloat(TIMER_VAR);
            Recipe* currentRecipe = storage.getPointer<Recipe>(CURRENTRECIPE_VAR);
            ItemStack inputStackOpt = storage.getStack(INPUTSTACK_VAR);
            ItemStack outputStackOpt = storage.getStack(OUTPUTSTACK_VAR);
            if(currentRecipe != nullptr) {
                craftStep(craftTimer,currentRecipe,inputStackOpt,outputStackOpt,dt);
            }
            storage.setFloat(FUEL_VAR,fuel);
            storage.setFloat(TIMER_VAR,craftTimer);
            storage.setStack(INPUTSTACK_VAR,inputStackOpt);
            storage.setStack(OUTPUTSTACK_VAR,outputStackOpt);
        }

        virtual std::vector<ItemStack> getDrops(Construction* construction,ivec3 position,BlockStorage& storage) {
            std::vector<ItemStack> drops = Block::getDrops(construction,position,storage);
            ItemStack inputStack = storage.getStack(INPUTSTACK_VAR);
            if(!inputStack.isEmpty()) drops.push_back(inputStack);
            ItemStack outputStack = storage.getStack(OUTPUTSTACK_VAR);
            if(!outputStack.isEmpty()) drops.push_back(outputStack);
            return drops;
        }

        bool trySetMatchingRecipe(Recipe*& currentRecipe,ItemStack& inputStack) {
            for(auto recipe : recipes) {
                if(inputStack.has(recipe->ingredients[0])) { //for just one ingredient
                    currentRecipe = recipe;
                    return true;
                }
            }
            return false;
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

        virtual void tryStartCraft(Recipe& recipe,Character& character,BlockStorage& storage) {
            if(!character.hasIngredients(recipe)) {
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
                character.take(recipe.ingredients[0]);
            } else {
                // try to take the item out
                if(!inputStack.isEmpty()) {
                    character.give(inputStack);
                    inputStack.clear();
                    // try again
                    if(inputStack.tryInsert(recipe.ingredients[0])) {
                        character.take(recipe.ingredients[0]);
                    }
                }
            }
            storage.setStack(INPUTSTACK_VAR,inputStack);
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,currentRecipe);
            

        }
};