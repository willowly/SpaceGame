#pragma once

#include <memory>

#include "block/block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"
#include "interface/block/block-menu-object.hpp"
#include "interface/block/block-widget.hpp"

class ContainerBlock : public Block {

    static const int INVENTORY_VAR = 0;

    static const int SIZE_VAR = 0;
public:
    
    float maxWeight = 50;
    BlockWidget<ContainerBlock>* widget = nullptr;
    TextureID texture = 0;

    ContainerBlock() : Block() {}

    StorageType getStorageType() override {
        return StorageType::Unique;
    }

    BlockStorage onPlace(Construction* construction, ivec3 position, BlockPlaceInfo placeInfo) override {
        BlockStorage storage;
        auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
        storage.clearStack(INVENTORY_VAR);
        return storage;
    }

    void onInteract(Construction* construction, ivec3 position, BlockStorage& storage, Character& character) override {
        if (widget == nullptr) {
            Debug::warn("container menu null");
            return;
        }

        auto menuObj = std::make_unique<BlockMenuObject<ContainerBlock>>(construction, position, *widget);
        character.openMenu(std::move(menuObj));
    }

    void addToMesh(Construction* construction, MeshData<ConstructionVertex>& meshData, ivec3 position, BlockStorage& storage) override {
        BlockHelper::addSingleBlock(construction,meshData,position,texture);
    }
    
    BlockInventoryInstance getInventory(BlockStorage& storage) {
        return BlockInventoryInstance(storage,INVENTORY_VAR,SIZE_VAR,maxWeight);
    }

    std::vector<ItemStack> getDrops(Construction* construction, ivec3 position, BlockStorage& storage) override {
        std::vector<ItemStack> drops = Block::getDrops(construction, position, storage);

        auto inventory = getInventory(storage);
        for(auto stack : inventory.getItems()) {
            if(stack == nullptr) continue;
            drops.push_back(*stack);
        }

        return drops;
    }

    ItemStack tryInsert(ivec3 position,BlockStorage& storage,BlockFacing direction,ItemStack stack) override {
        auto inventory = getInventory(storage);
        stack = inventory.tryGive(stack);
        return stack;
    }

    bool canInsert() override {
        return true;
    }

    string getTypeName() override {
        return "container";
    }
};