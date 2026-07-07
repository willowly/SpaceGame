#pragma once

#include "block.hpp"
#include "actor/construction.hpp"

#include "helper/block-helper.hpp"
#include "graphics/basic-model.hpp"

#include "block/display/cube-block-display.hpp"

class ThrusterBlock : public Block
{
public:
    ThrusterBlock() : Block()
    {
    }

    // ints
    static const int FACING_VAR = 0;

    // displays
    static const int DISPLAY_VAR = 0;

    BasicModel cubeModel;

    Mesh<Vertex> *mesh = nullptr;
    TextureID texture = 0;
    float force;     // for now this is always forwards
    float sideForce; // for now this is always the 4 directions parrellel to the back

    StorageType getStorageType() override
    {
        return StorageType::Unique;
    }

    BlockStorage onPlace(Construction *construction, ivec3 position, BlockPlaceInfo placeInfo) override
    {
        BlockStorage storage;
        auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
        storage.setFacing(FACING_VAR, facing);
        auto display = construction->addBlockDisplay(std::make_unique<BlockCubeDisplay>(cubeModel,1,position,BlockHelper::getRotationFromFacing(facing)));
        construction->addStepCallback(position);
        storage.setBlockDisplay(DISPLAY_VAR,display);
        return storage;
    }

    void onStep(World *world, Construction *construction, ivec3 position, BlockStorage &storage, float dt) override
    {
        auto moveControl = construction->getMoveControl();
        auto facing = storage.getFacing(FACING_VAR);
        auto rotation = BlockHelper::getRotationFromFacing(facing);

        moveControl = glm::inverse(rotation) * moveControl; // transform moveControl into thruster-local space

        vec3 localForce = {};
        auto display = storage.getBlockDisplay(DISPLAY_VAR);
        display->storage.setFloat(BlockCubeDisplay::SPEED_VAR,1);
        if (moveControl.z < -0.01) //because we are pushing the craft backwards
        {
            localForce += vec3(0,0,1) *   force   * moveControl.z;
            display->storage.setFloat(BlockCubeDisplay::SPEED_VAR,5);
        }
        if(moveControl.x < 0.01) {
            localForce += vec3(1,0,0) * sideForce * moveControl.x;
        }
        if(moveControl.x > 0.01) {
            localForce += vec3(1,0,0) * sideForce * moveControl.x;
        }
        if(moveControl.y < 0.01) {
            localForce += vec3(0,1,0) * sideForce * moveControl.y;
        }
        if(moveControl.y > 0.01) {
            localForce += vec3(0,1,0) * sideForce * moveControl.y;
        }
        localForce = rotation * localForce;
        construction->applyForce(localForce);
    }

    virtual void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) {
        auto display = storage.getBlockDisplay(DISPLAY_VAR);
        construction->removeBlockDisplay(display);
    }

    // void addThrustForces(Construction* construction,BlockFacing facing) {
    //     construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::UP,facing))] += sideForce;
    //     construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::DOWN,facing))] += sideForce;
    //     construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::RIGHT,facing))] += sideForce;
    //     construction->thrustForces[static_cast<int>(BlockHelper::rotateFacingByFacing(BlockFacing::LEFT,facing))] += sideForce;
    //     construction->thrustForces[static_cast<int>(facing)] += force;
    // }

    virtual void addToMesh(Construction *construction, MeshData<ConstructionVertex> &meshData, ivec3 position, BlockStorage &storage)
    {
        BlockHelper::addMesh(meshData, position, storage.getFacing(FACING_VAR), mesh, texture);
    }

    string getTypeName() override
    {
        return "thruster";
    }
};