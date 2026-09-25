#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"

class BatteryBlock : public Block {
    public:

        BatteryBlock() : Block() {

        }

        float maxCharge;

        // ints
        static const int FACING_VAR = 0;
        //static const int ROTATION_VAR = 1;
        TextureID texture = {};
        BlockWidget<BatteryBlock>* widget = {};

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            storage.setFacing(FACING_VAR,facing);
            return storage;
        }

        void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) {
            construction->getNetwork(0).changeMaxCharge(maxCharge);
        }


        void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) override {
            construction->getNetwork(0).changeMaxCharge(-maxCharge);
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            quat rotation = BlockHelper::getRotationFromFacing(storage.getFacing(FACING_VAR)) * glm::quat(glm::radians(vec3(90.0f,0.0f,0.0f)));
            BlockHelper::addSingleBlock(construction,meshData,position,texture,glm::identity<quat>());
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {
            Debug::lua(std::to_string(construction->getNetwork(0).getCurrentCharge()) + "/" + std::to_string(construction->getNetwork(0).getMaxCharge()));
        }

        void onLook(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) override {
            auto menuObj = std::make_unique<BlockMenuObject<BatteryBlock>>(construction->id, position, *widget);
            character.setPreview(std::move(menuObj));
        }


        string getTypeName() override {
            return "battery";
        }
};