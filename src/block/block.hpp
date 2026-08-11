#pragma once


#include <graphics/mesh.hpp>
#include <item/item-stack.hpp>
#include "block-facing.hpp"
#include "helper/block-storage.hpp"
#include "engine/object.hpp"


class Construction;
struct ConstructionVertex;
class Character;


struct BlockPlaceInfo {
    vec3 normal = {}; //local to the construction
    vec3 lookDir = {};
    vec3 upDir = vec3(0,1,0);
    bool attached = false; //is this block being placed attached to terrain?
};

class Block : public Object {

    

    public:

        enum class ModelType { //scoped enum to avoid name conflicts
            SingleBlock,
            ConnectedBlock,
            Mesh
        };

        enum class StorageType {
            Constant,
            Unique
        };

        Item* defaultDrop = nullptr; // everthing gets turned into functions...
        bool solid = false; // should probably also be a function just unsure rn

        struct PlacementData {
            vec3 normal = {};
        };



    
        Block() {}
        
        virtual ~Block() = default;

        virtual StorageType getStorageType() {
            return StorageType::Constant;
        }

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) { //im unsure how to handle placement of different block states
            return BlockStorage();
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) {

        }

        // when this block is loaded. Happens when placed and when loaded from a save
        virtual void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) {

        }

        virtual void onHammer(Construction* construction,vec3 hammerPosition,ivec3 blockPosition,BlockStorage& storage) {

        }

        // needs to use construction.addStepCallback() to make work
        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) {

        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {

        }

        virtual std::vector<ItemStack> getDrops(Construction* construction,ivec3 position,BlockStorage& storage) {
            if(defaultDrop == nullptr) {
                return std::vector<ItemStack>();
            } else {
                return std::vector<ItemStack> {ItemStack(defaultDrop,1)};
            }
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {}

        string getTypeName() override {
            return "block";
        }

};