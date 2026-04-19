#define GLM_ENABLE_EXPERIMENTAL
#include "engine/world.hpp"
#include "actor/construction.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Construction", "[construction]") {

    World world;
    auto construction = world.spawn(Construction::makeInstance(Material::none,vec3(0)));
     

    SECTION( "can get empty blocks" , "[can get empty blocks]") {
        INFO("Get empty blocks");
        REQUIRE(construction->getBlock(vec3(0)).block == nullptr);
        REQUIRE(construction->getBlock(vec3(1,-5,1)).block == nullptr);
        REQUIRE(construction->getBlock(vec3(10,0,-2)).block == nullptr);

    }

    Block testBlock;

    testBlock.name = "test";


    SECTION( "can place block" , "[can place a block and get that block]") {
        INFO("Place block");
        construction->placeBlock(vec3(0),&testBlock);

        REQUIRE(construction->getBlock(vec3(0)).block == &testBlock);

    }

    SECTION( "can break block") {

       
        construction->placeBlock(vec3(0),&testBlock);
        construction->placeBlock(vec3(1,0,0),&testBlock);
        construction->placeBlock(vec3(1,1,0),&testBlock);
        construction->placeBlock(vec3(-1,0,0),&testBlock);
        construction->placeBlock(vec3(-1,1,0),&testBlock);

        construction->breakBlock(&world,vec3(0));


        auto constructions = world.getActorsOfType<Construction>();

        REQUIRE(constructions.size() == 2);

        for (size_t i = 0; i < constructions.size(); i++)
        {
            auto pair = constructions[i]->getBounds();
            REQUIRE((pair.second - pair.first) == ivec3(0,1,0));
            REQUIRE(constructions[i]->getBlock(ivec3(0,0,0) + pair.first).block == &testBlock); 
            REQUIRE(constructions[i]->getBlock(ivec3(0,1,0) + pair.first).block == &testBlock); 
        }

    }
}