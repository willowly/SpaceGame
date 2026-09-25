#pragma once

#include "block/block.hpp"

#include "helper/block-storage.hpp"

class Crafter
{
    //ints
    int RUNNING_VAR;

    //floats
    int TIMER_VAR;

    //stacks
    int OUTPUTSTACK_VAR;
    std::vector<int> INPUTSTACKS_VAR;

    //pointers
    int CURRENTRECIPE_VAR;

     public:
        explicit Crafter(int inputStacks,BlockStorageVarInitalizer &vars)
        {
            RUNNING_VAR = vars.getNextInt();
            TIMER_VAR = vars.getNextFloat();
            OUTPUTSTACK_VAR = vars.getNextStack();
            for (size_t i = 0; i < inputStacks; i++)
            {
                INPUTSTACKS_VAR.push_back(vars.getNextStack());
            }
            
            CURRENTRECIPE_VAR = vars.getNextPointer();
        }
}