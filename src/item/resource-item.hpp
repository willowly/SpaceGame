#pragma once
#include "item.hpp"
#include "graphics/vulkan.hpp"


class ResourceItem : public Item {

    public:
        TextureID icon;

        virtual TextureID getIcon() {
            return icon;
        }


};