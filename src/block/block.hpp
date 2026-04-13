#pragma once


#include <graphics/mesh.hpp>
#include <item/item-stack.hpp>
#include "block-facing.hpp"
#include "helper/block-storage.hpp"


class Construction;
struct ConstructionVertex;
class Character;




class Block {

    

    public:

        enum class ModelType { //scoped enum to avoid name conflicts
            SingleBlock,
            ConnectedBlock,
            Mesh
        };

        string name; // for prototypes
        Item* drop; // everthing gets turned into functions...

        struct PlacementData {
            vec3 normal = {};
        };

        bool solid = false; // should probably also be a function just unsure rn

    
        Block() {}
        
        virtual ~Block() = default;

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockFacing facing) { //im unsure how to handle placement of different block states
            return BlockStorage();
        }

        virtual void onBreak(Construction* construction,ivec3 position,BlockStorage& storage) {

        }

        // when this block is loaded
        virtual void onLoad(Construction* construction,ivec3 position,BlockStorage& storage) {

        }

        // needs to use construction.addStepCallback() to make work
        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) {

        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {

        }

        virtual std::vector<ItemStack> getDrops(Construction* construction,ivec3 position,BlockStorage& storage) {
            if(drop == nullptr) {
                return std::vector<ItemStack>();
            } else {
                return std::vector<ItemStack> {ItemStack(drop,1)};
            }
        }

        virtual void onInteract(Construction* construction,ivec3 position,BlockStorage& storage,Character& character) {}

};