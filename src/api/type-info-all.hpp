#pragma once
#include "type-info.hpp"
#include "api/type-info-item.hpp"
#include "api/type-info-block.hpp"
#include "api/type-info-graphics.hpp"
#include "api/type-info-math.hpp"
#include "api/type-info-widget.hpp"


namespace TypeInfoLoader {

    inline void loadAll(Registry& registry) {

        TypeInfoLoader::loadBlock(registry);
        TypeInfoLoader::loadItem(registry);
        TypeInfoLoader::loadGraphics(registry);
        TypeInfoLoader::loadMath(registry);
        TypeInfoLoader::loadWidget(registry);

    }

}