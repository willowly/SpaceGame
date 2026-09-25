#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"

class SolarPanelBlock : public Block {
    public:

        SolarPanelBlock() : Block() {

        }

        float powerOutput;
        float maxCharge;

        // ints
        static const int FACING_VAR = 0;
        //static const int ROTATION_VAR = 1;

        Mesh<Vertex>* mesh = {};
        TextureID texture = {};

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            
            auto blockEntry = BlockHelper::getBlockPlacedOn(construction,position,placeInfo);
            if(blockEntry.block == this) {
                auto facing = blockEntry.storage.getFacing(FACING_VAR);
                storage.setFacing(FACING_VAR,facing);
            } else {
                auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
                storage.setFacing(FACING_VAR,facing);

            }

            construction->addStepCallback(position);
            return storage;
        }

        void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) override {
            construction->getNetwork(0).changeMaxCharge(maxCharge);
        }

        void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) override {
            construction->getNetwork(0).changeCurrentCharge(powerOutput * dt);
        }

        void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) override {
            construction->getNetwork(0).changeMaxCharge(-maxCharge);
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            assert(mesh != nullptr);
            quat rotation = BlockHelper::getRotationFromFacing(storage.getFacing(FACING_VAR)) * glm::quat(glm::radians(vec3(90.0f,0.0f,0.0f)));
            BlockHelper::addMesh(meshData,position,rotation,mesh->meshData,texture);
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            Debug::lua(std::to_string(construction->getNetwork(0).getCurrentCharge()) + "/" + std::to_string(construction->getNetwork(0).getMaxCharge()));
        }


        string getTypeName() override {
            return "solar_panel";
        }
};