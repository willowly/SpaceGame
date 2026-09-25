#pragma once

#include "block/block.hpp"

#include "helper/block-storage.hpp"

class FuelBurner
{
    int FUEL_VAR;
    int FUEL_MAX_VAR;
    int FUELSTACK_VAR;
    
    
    public:
        explicit FuelBurner(BlockStorageVarInitalizer &vars)
        {
            FUEL_VAR = vars.getNextFloat();
            FUEL_MAX_VAR = vars.getNextFloat();
            FUELSTACK_VAR = vars.getNextStack();
        }
        float burnSpeed = 1;
        bool smartFuel = false;
        bool powerStep(bool running,BlockStorage& storage,float dt) {

            bool consumeMoreFuel = !smartFuel || running;

            auto fuel = storage.getFloat(FUEL_VAR);
            auto fuelMax = storage.getFloat(FUEL_MAX_VAR);
            auto fuelStack = storage.getStack(FUELSTACK_VAR);

            if(fuel <= 0) {
                if(consumeMoreFuel && !fuelStack.isEmpty() && fuelStack.item->fuelValue > 0) {
                    fuel = fuelStack.item->fuelValue;
                    fuelMax = fuelStack.item->fuelValue;
                    fuelStack.amount--;
                }
            }
            fuel -= dt * burnSpeed;

            storage.setFloat(FUEL_VAR,fuel);
            storage.setFloat(FUEL_MAX_VAR,fuelMax);
            storage.setStack(FUELSTACK_VAR,fuelStack);
            return fuel > 0;
            

            
        }

        ItemStack getFuelStack(BlockStorage& storage) {
            return storage.getStack(FUELSTACK_VAR);
        }

        void setFuelStack(BlockStorage& storage,ItemStack stack) {
            return storage.setStack(FUELSTACK_VAR,stack);
        }

        float getFuel(BlockStorage& storage) {
            return storage.getFloat(FUEL_VAR);
        }

        float getFuelMax(BlockStorage& storage) {
            return storage.getFloat(FUEL_MAX_VAR);
        }
};