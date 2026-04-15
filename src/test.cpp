#define GLM_ENABLE_EXPERIMENTAL
#include "engine/world.hpp"
#include "actor/construction.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Hello Message", "[hello_message]") {
  
    auto construction = Construction::makeInstance(Material::none,vec3(0));

    REQUIRE(construction->getBlock(vec3(0)).block == nullptr);
}