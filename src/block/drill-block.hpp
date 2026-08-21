#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"
#include "block/display/model-block-display.hpp"

class DrillBlock : public Block {
    public:

        DrillBlock() : Block() {

        }

        // ints
        static const int FACING_VAR = 0;
        static const int INVENTORY_SIZE_VAR = 1;

        // items
        static const int INVENTORY_VAR = 0;


        //block display
        static const int BLOCK_DISPLAY = 0;

        BlockWidget<DrillBlock>* widget = nullptr;

        Mesh<Vertex>* mesh = {};
        TextureID texture = {};
        quat meshRotation = {};

        BasicModel drillHeadModel = {};

        float drillRotateSpeed = 5;

        float maxWeight = 10;

        float range = 1;
        float amount = 0.1;
        float radius = 1;

        StorageType getStorageType() override {
            return StorageType::Unique;
        }

        BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) override {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            construction->addStepCallback(position);
            storage.setFacing(FACING_VAR,facing);
            onLoad(construction,position,storage);
            return storage;
        }

        void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) override {
            auto facing = storage.getFacing(FACING_VAR);
            auto display = construction->addBlockDisplay(std::make_unique<ModelBlockDisplay>(drillHeadModel,15,position,BlockHelper::getRotationFromFacing(facing)));
            display->storage.setInt(ModelBlockDisplay::SPEED_VAR,drillRotateSpeed);
            storage.setBlockDisplay(BLOCK_DISPLAY,display);
        }

        void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) override {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            BlockHelper::addMesh(meshData,position,facing,mesh,texture);
        }

        void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) override {

            if(Random::value() > 0.1f) return;

            BlockFacing facing = storage.getFacing(FACING_VAR);
            Ray ray = {construction->transformPoint(position),construction->transformDirection(vec3(0,0,1) * BlockHelper::getRotationFromFacing(facing))};
            ray.origin += ray.direction*0.55f;
            auto hitOpt = world->raycast(ray,range,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));



            auto inventory = getInventory(storage);

            if(hitOpt) {
                auto hit = hitOpt.value();
                Terrain* terrain = dynamic_cast<Terrain*>(hit.actor);
                if(terrain != nullptr) {
                    
                    int spaceLeft = inventory.getSpaceLeft();
                    bool full = spaceLeft <= 0;
                    auto results = terrain->terraformSphere(world,hit.point,radius,-amount,full); //if its full, just spawn items like normal
                    if(!full) {
                        for(auto stack : results.items) {
                            auto remaining = inventory.tryGive(stack);
                            if(!remaining.isEmpty()) world->spawn(ItemActor::makeInstance(remaining,hit.point));
                        }
                    }
                }
            }

            auto insertPosition = position + BlockHelper::rotateByFacing(ivec3(0,0,-1),facing);
            auto& entry = construction->getBlock(position + BlockHelper::rotateByFacing(ivec3(0,0,-1),facing));
            if(entry.block != nullptr && entry.block->canInsert()) {
                auto stackToInsert = inventory.getFirstItem();
                if(!stackToInsert.isEmpty()) {
                    stackToInsert.amount = 1;
                    stackToInsert.amount = inventory.take(stackToInsert);
                    auto remaining = entry.block->tryInsert(position,entry.storage,BlockHelper::rotateFacingByFacing(BlockFacing::BACKWARD,facing),stackToInsert);
                    inventory.give(remaining);
                }
                
            }

        }

        BlockInventoryInstance getInventory(BlockStorage& storage) {
            return BlockInventoryInstance(storage,INVENTORY_VAR,INVENTORY_SIZE_VAR,maxWeight);
        }

        void onInteract(Construction* construction, ivec3 position, BlockStorage& storage, Character& character) override {
            if (widget == nullptr) {
                Debug::warn("container menu null");
                return;
            }

            auto menuObj = std::make_unique<BlockMenuObject<DrillBlock>>(construction, position, *widget);
            character.openMenu(std::move(menuObj));
        }

        void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) override {
            auto display = storage.getBlockDisplay(BLOCK_DISPLAY);
            construction->removeBlockDisplay(display);
        }

        string getTypeName() override {
            return "drill";
        }
};