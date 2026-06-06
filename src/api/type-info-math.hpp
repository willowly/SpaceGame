#pragma once
#include "type-info.hpp"
#include "graphics/vulkan.hpp"

#include <glm/glm.hpp>


namespace TypeInfoLoader {

    inline void loadMath(Registry& registry) {
        
        TypeInfo* vec2 = registry.addTypeInfo<glm::vec2>("vec2");
        vec2->addConstProperty("x",&glm::vec2::x);
        vec2->addConstProperty("y",&glm::vec2::y);

        TypeInfo* vec3 = registry.addTypeInfo<glm::vec3>("vec3");
        vec3->addConstProperty("x",&glm::vec3::x);
        vec3->addConstProperty("y",&glm::vec3::y);
        vec3->addConstProperty("z",&glm::vec3::z);

        TypeInfo* vec4 = registry.addTypeInfo<glm::vec4>("vec4");
        vec4->addConstProperty("x",&glm::vec4::x);
        vec4->addConstProperty("y",&glm::vec4::y);
        vec4->addConstProperty("z",&glm::vec4::z);
        vec4->addConstProperty("w",&glm::vec4::w);

        TypeInfo* ivec2 = registry.addTypeInfo<glm::ivec2>("ivec2");
        ivec2->addConstProperty("x",&glm::ivec2::x);
        ivec2->addConstProperty("y",&glm::ivec2::y);

        TypeInfo* ivec3 = registry.addTypeInfo<glm::ivec3>("ivec3");
        ivec3->addConstProperty("x",&glm::ivec3::x);
        ivec3->addConstProperty("y",&glm::ivec3::y);
        ivec3->addConstProperty("z",&glm::ivec3::z);

        TypeInfo* ivec4 = registry.addTypeInfo<glm::ivec4>("ivec4");
        ivec4->addConstProperty("x",&glm::ivec4::x);
        ivec4->addConstProperty("y",&glm::ivec4::y);
        ivec4->addConstProperty("z",&glm::ivec4::z);
        ivec4->addConstProperty("w",&glm::ivec4::w);

        TypeInfo* quat = registry.addTypeInfo<glm::quat>("quat");
        quat->addConstProperty("x",&glm::quat::x);
        quat->addConstProperty("y",&glm::quat::y);
        quat->addConstProperty("z",&glm::quat::z);
        quat->addConstProperty("w",&glm::quat::w);

        TypeInfo* rect = registry.addTypeInfo<Rect>("rect");
        rect->addConstProperty("position",&Rect::position);
        rect->addConstProperty("size",&Rect::position);
        
    }

}