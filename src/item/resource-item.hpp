#pragma once
#include "item.hpp"
#include "graphics/vulkan.hpp"
#include "helper/sprite.hpp"


class ResourceItem : public Item {

    public:
        Sprite icon;

        virtual Sprite getIcon() {
            return icon;
        }


};