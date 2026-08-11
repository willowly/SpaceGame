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

    static const int MAX_SLOT = 0;
public:
    

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

    void setStack(BlockStorage& storage,ItemStack stack,int slot) {
        int maxSlot = getMaxSlot(storage);
        storage.setStack(ContainerBlock::INVENTORY_VAR+slot,stack);
        std::vector<ItemStack> inventory;
        // for (size_t i = 0; i < maxSlot; i++)
        // {
        //     inventory.push_back(storage.getStack())
        // }
        
    }

    int getMaxSlot(BlockStorage& storage) {
        return storage.getInt(ContainerBlock::MAX_SLOT);
    }

    ItemStack getStack(BlockStorage& storage,int slot) {
        return storage.getStack(ContainerBlock::INVENTORY_VAR+slot);
    }

    std::vector<ItemStack> getDrops(Construction* construction, ivec3 position, BlockStorage& storage) override {
        std::vector<ItemStack> drops = Block::getDrops(construction, position, storage);

        ItemStack inventory = storage.getStack(INVENTORY_VAR);
        if (!inventory.isEmpty()) {
            drops.push_back(inventory);
        }

        return drops;
    }

    string getTypeName() override {
        return "container";
    }
};