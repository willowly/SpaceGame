#pragma once
#include "graphics/vulkan.hpp"
#include "helper/generic-storage.hpp"

class Construction;

class BlockDisplay {

    protected:
        vec3 position;
        quat rotation;

    public:
        GenericStorage storage;

        BlockDisplay(ivec3 position,quat rotation) : position(position), rotation(rotation) {
            
        }

        virtual void addRenderables(Vulkan* vulkan,Construction* construction,float dt,float interpolation) = 0;

};