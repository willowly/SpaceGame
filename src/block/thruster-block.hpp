#pragma once

#include "block.hpp"
#include "actor/construction.hpp"

#include "helper/block-helper.hpp"
#include "graphics/basic-model.hpp"

#include "block/display/particle-block-display.hpp"

class ThrusterBlock : public Block
{


    private:

        void addParticleDisplay(Construction* construction,vec3 position,BlockStorage& storage,int displayVar,ParticleEffectSettings* settings,quat rotation) {
            auto display = construction->addBlockDisplay(std::make_unique<ParticleBlockDisplay>(settings,position,rotation));
            storage.setBlockDisplay(displayVar,display);
        } 

    public:
        ThrusterBlock() : Block()
        {
        }

        // ints
        static const int FACING_VAR = 0;

        // displays
        static const int DISPLAY_VAR = 0;
        static const int R_DISPLAY_VAR = 1;
        static const int L_DISPLAY_VAR = 2;
        static const int U_DISPLAY_VAR = 3;
        static const int D_DISPLAY_VAR = 4;

        ParticleEffectSettings thrustEffect;
        ParticleEffectSettings smallThrustEffect;
        float thrustEffectDistance = 0.5f;

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
            construction->addStepCallback(position);
            return storage;
        }

        void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) override {
            auto facing = storage.getFacing(FACING_VAR);
            auto rotation = BlockHelper::getRotationFromFacing(facing);
            auto display = construction->addBlockDisplay(std::make_unique<ParticleBlockDisplay>(&thrustEffect,position,rotation));
            storage.setBlockDisplay(DISPLAY_VAR,display);
            if(sideForce > 0) {
                addParticleDisplay(construction,position,storage,R_DISPLAY_VAR,&smallThrustEffect,BlockHelper::getRotationFromFacing(facing) * BlockHelper::getRotationFromFacing(BlockFacing::LEFT));
                addParticleDisplay(construction,position,storage,L_DISPLAY_VAR,&smallThrustEffect,BlockHelper::getRotationFromFacing(facing) * BlockHelper::getRotationFromFacing(BlockFacing::RIGHT));
                addParticleDisplay(construction,position,storage,U_DISPLAY_VAR,&smallThrustEffect,BlockHelper::getRotationFromFacing(facing) * BlockHelper::getRotationFromFacing(BlockFacing::DOWN));
                addParticleDisplay(construction,position,storage,D_DISPLAY_VAR,&smallThrustEffect,BlockHelper::getRotationFromFacing(facing) * BlockHelper::getRotationFromFacing(BlockFacing::UP));
            }
        }

        void onStep(World *world, Construction *construction, ivec3 position, BlockStorage &storage, float dt) override
        {
            auto moveControl = construction->getMoveControl();
            auto facing = storage.getFacing(FACING_VAR);
            auto rotation = BlockHelper::getRotationFromFacing(facing);

            moveControl = glm::inverse(rotation) * moveControl; // transform moveControl into thruster-local space

            vec3 localForce = {};
            auto display = storage.getBlockDisplay(DISPLAY_VAR);
            display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,0);
            if (moveControl.z < -0.01) //because we are pushing the craft backwards
            {
                localForce += vec3(0,0,1) *   force   * moveControl.z;
                display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,-moveControl.z);
            }
            if(sideForce > 0) {

                auto r_display = storage.getBlockDisplay(R_DISPLAY_VAR);
                auto l_display = storage.getBlockDisplay(L_DISPLAY_VAR);
                auto u_display = storage.getBlockDisplay(U_DISPLAY_VAR);
                auto d_display = storage.getBlockDisplay(D_DISPLAY_VAR);
                r_display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,moveControl.x);
                l_display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,-moveControl.x);
                u_display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,moveControl.y);
                d_display->storage.setFloat(ParticleBlockDisplay::EMISSION_VAR,-moveControl.y);

                if(moveControl.x < -0.01) {
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