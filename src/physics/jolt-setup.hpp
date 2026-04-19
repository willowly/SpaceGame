#include <Jolt/Jolt.h>
#include "physics/jolt-terrain-shape.hpp"

namespace Physics {

    static bool physicsInitalized = false;


    static void initalizePhysicsGlobal() {

        if(physicsInitalized) return;
        
        JPH::RegisterDefaultAllocator(); //we use the default allocator for jolt
        JPH::Factory::sInstance = new JPH::Factory(); //set the factory singleton
        JPH::RegisterTypes(); //idfk
        TerrainShape::sRegister();
        physicsInitalized = true;
    }

}