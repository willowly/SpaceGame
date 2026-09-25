#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"
#include "interface/block/block-menu-object.hpp"
#include "interface/block/block-widget.hpp"
#include "item/recipe.hpp"

#include "block/component/fuel-burner.hpp"



class CrafterBlock : public Block {

        
    BlockStorageVarInitalizer vars;

    public:

        //ints
        int FACING_VAR;
        int ROTATION_VAR;
        int RUNNING_VAR;

        //floats
        int TIMER_VAR;

        //stacks
        int OUTPUTSTACK_VAR;
        int INPUTSTACK_VAR;

        //pointers
        int CURRENTRECIPE_VAR;

        string category;
        int maxIngredients = 1;

        bool electric = false;
        float electricUseSpeed = 0;

        float craftSpeed = 1;

        bool allowManual;
        float manualProgress = 0.5f;

        FuelBurner fuelBurner;

        BlockWidget<CrafterBlock>* widget = {};
        std::vector<Recipe*> recipes;

        Mesh<Vertex>* mesh = nullptr;
        TextureID texture = 0;
    
        CrafterBlock() : Block(), fuelBurner(vars) {
            
            FACING_VAR = vars.getNextInt();
            ROTATION_VAR = vars.getNextInt();
            RUNNING_VAR = vars.getNextInt();

            TIMER_VAR = vars.getNextFloat();

            OUTPUTSTACK_VAR = vars.getNextStack();
            
            CURRENTRECIPE_VAR = vars.getNextPointer();

            INPUTSTACK_VAR = vars.getNextStack(); 

        }

        StorageType getStorageType() override {
            return StorageType::Unique;
        }

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            
            
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            construction->addStepCallback(position);

            vec2 up = glm::inverse(BlockHelper::getRotationFromFacing(facing)) * placeInfo.lookDir;
            int rotation = BlockHelper::getRotationIndexFromVector(up);

            if(placeInfo.firstBlock) {
                facing = BlockFacing::UP;
                rotation = 0;
            }

            storage.setFacing(FACING_VAR,facing);
            storage.setInt(ROTATION_VAR,rotation);
            storage.setInt(RUNNING_VAR,0);
            storage.clearStack(OUTPUTSTACK_VAR);
            for (size_t i = 0; i < maxIngredients; i++)
            {
                storage.clearStack(INPUTSTACK_VAR+i);
            }
            
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,nullptr);
            return storage;
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            if(widget == nullptr) {
                Debug::warn("furnace menu null");
                return;
            }
            auto menuObj = std::make_unique<BlockMenuObject<CrafterBlock>>(construction->id,position,*widget);
            character.openMenu(std::move(menuObj));
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            quat rotation = BlockHelper::getRotationFromFacing(storage.getFacing(FACING_VAR),storage.getInt(ROTATION_VAR)) * glm::quat(glm::radians(vec3(90.0f,0.0f,0.0f)));
            BlockHelper::addMesh(meshData,position,rotation,mesh->meshData,texture);
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) {

            
            
            craftStep(construction,position,storage,dt);
        }

        virtual std::vector<ItemStack> getDrops(Construction* construction,ivec3 position,BlockStorage& storage) {
            std::vector<ItemStack> drops = Block::getDrops(construction,position,storage);
            
            ItemStack outputStack = storage.getStack(OUTPUTSTACK_VAR);
            if(!outputStack.isEmpty()) drops.push_back(outputStack);
            ItemStack fuelStack =  fuelBurner.getFuelStack(storage);
            if(!fuelStack.isEmpty()) drops.push_back(fuelStack);

            for (size_t i = 0; i < maxIngredients; i++)
            {
                ItemStack inputStack = storage.getStack(INPUTSTACK_VAR);
                if(!inputStack.isEmpty()) drops.push_back(inputStack);
            }
            
            return drops;
        }

        bool trySetMatchingRecipe(Recipe*& currentRecipe,BlockStorage& storage) {
            for(auto recipe : recipes) {
                if(hasIngredients(recipe,storage)) {
                    currentRecipe = recipe;
                    return true;
                }
            }
            return false;
        }

        bool electricPowerStep(bool running,Construction* construction,BlockStorage& storage,float dt) {
            if(!running) {
                return false;
            }
            construction->getNetwork(0).changeCurrentCharge(-electricUseSpeed * dt);
            return construction->getNetwork(0).getCurrentCharge() > 0;
        }

        bool hasIngredients(Recipe* recipe,BlockStorage& storage) {
            if(recipe == nullptr) {
                return false;
            }
            for (size_t i = 0; i < maxIngredients; i++)
            {
                if(i > recipe->ingredients.size()-1) {
                    if(!storage.getStack(INPUTSTACK_VAR+i).isEmpty()) {
                        return false;
                    }
                    continue;
                }
                if(!storage.getStack(INPUTSTACK_VAR+i).has(recipe->ingredients[i])) {
                    return false;
                }
            }
            return true;
        }

        void craftStep(Construction* construction,ivec3 position,BlockStorage& storage,float dt) {
            
            bool running = storage.getInt(RUNNING_VAR) ? 1 : 0;
            bool powerOn = false;
            
            if(electric) {
                powerOn = electricPowerStep(running,construction,storage,dt);
            } else {
                powerOn = fuelBurner.powerStep(running,storage,dt);
            }


            float craftTimer = storage.getFloat(TIMER_VAR);
            Recipe* currentRecipe = storage.getPointer<Recipe>(CURRENTRECIPE_VAR);
            ItemStack outputStack = storage.getStack(OUTPUTSTACK_VAR);

            if(currentRecipe == nullptr) {
                running = false;
                return;
            }

            if(currentRecipe->ingredients.size() <= maxIngredients) {
                
                if(!hasIngredients(currentRecipe,storage)) {
                    craftTimer = 0;
                    trySetMatchingRecipe(currentRecipe,storage);
                    running = false;
                    storage.setInt(RUNNING_VAR,running);
                    storage.setFloat(TIMER_VAR,craftTimer);
                    storage.setPointer<Recipe>(CURRENTRECIPE_VAR,currentRecipe);
                    return;
                }
                running = true;

                if(powerOn) {
                    craftTimer += dt * craftSpeed;
                }
                if(craftTimer >= currentRecipe->time) {
                    
                    if(hasIngredients(currentRecipe,storage)) {
                        if(outputStack.tryInsert(currentRecipe->result)) {
                            for (size_t i = 0; i < currentRecipe->ingredients.size(); i++)
                            {
                                storage.getStack(INPUTSTACK_VAR+i).amount -= currentRecipe->ingredients[i].amount;
                            }
                            
                            craftTimer = 0;
                        }
                    }
                }
            } else {
                Debug::warn("crafter recipe has " + std::to_string(currentRecipe->ingredients.size()) + " ingredients (must be " + std::to_string(maxIngredients) + ")");
                return;
            }

            storage.setInt(RUNNING_VAR,running);
            // storage.setFloat(FUEL_VAR,fuel);
            // storage.setFloat(FUEL_MAX_VAR,fuelMax);
            storage.setFloat(TIMER_VAR,craftTimer);
            storage.setStack(OUTPUTSTACK_VAR,outputStack);
        }

        void progressManual(BlockStorage& storage) {
            Recipe* currentRecipe = storage.getPointer<Recipe>(CURRENTRECIPE_VAR);
            if(hasIngredients(currentRecipe,storage)) {
                float progress = storage.getFloat(TIMER_VAR);
                progress += manualProgress;
                storage.setFloat(TIMER_VAR,progress);
            }
        }

        virtual void tryStartCraft(Recipe& recipe,Character& character,BlockStorage& storage) {
            if(!character.hasIngredients(recipe)) {
                return;
            }
            if(recipe.ingredients.size() > maxIngredients) {
                Debug::warn("crafter recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be " + std::to_string(maxIngredients) + ")");
                return;
            }
             //should have them so we dont need to check

            auto currentRecipe = storage.getPointer<Recipe>(CURRENTRECIPE_VAR);
            if(currentRecipe != &recipe) {
                storage.setFloat(TIMER_VAR,0);
                currentRecipe = &recipe;
            }

            for (size_t i = 0; i < recipe.ingredients.size(); i++)
            {
                auto& inputStack = storage.getStack(INPUTSTACK_VAR+i);
                if(inputStack.tryInsert(recipe.ingredients[i])) {
                    character.take(recipe.ingredients[i]);
                } else {
                    // try to take the item out
                    if(!inputStack.isEmpty()) {
                        character.give(inputStack);
                        inputStack.clear();
                        // try again
                        if(inputStack.tryInsert(recipe.ingredients[i])) {
                            character.take(recipe.ingredients[i]);
                        }
                    }
                }
            }
            
            
            storage.setPointer<Recipe>(CURRENTRECIPE_VAR,currentRecipe);
            

        }

        virtual void getRecipes(std::function<void(std::vector<Recipe*>&,RecipeFilter)> getRecipesFunction) {
            getRecipesFunction(recipes,RecipeFilter{
                .category = category,
                .maxIngredients = maxIngredients}
            );
        }

        string getTypeName() override {
            return "crafter";
        }
};