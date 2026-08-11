#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "actor/character.hpp"
#include "helper/block-helper.hpp"

class DrillBlock : public Block {
    public:

        DrillBlock() : Block() {

        }

        // ints
        static const int FACING_VAR = 0;

        Mesh<Vertex>* mesh = {};
        TextureID texture = {};
        quat meshRotation = {};

        float range = 1;
        float amount= 0.1;
        float radius = 1;

        virtual BlockStorage onPlace(Construction* construction,ivec3 position,BlockPlaceInfo placeInfo) {
            BlockStorage storage;
            auto facing = BlockHelper::getFacingFromVector(placeInfo.normal);
            construction->addStepCallback(position);
            storage.setFacing(FACING_VAR,facing);
            return storage;
        }

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockFacing facing = storage.getFacing(FACING_VAR);
            BlockHelper::addMesh(meshData,position,facing,mesh,texture);
        }

        virtual void onStep(World* world,Construction* construction,ivec3 position,BlockStorage& storage,float dt) {

            if(Random::value() > 0.1f) return;


            BlockFacing facing = storage.getFacing(FACING_VAR);
            Ray ray = {construction->transformPoint(position),construction->transformDirection(vec3(0,0,1) * BlockHelper::getRotationFromFacing(facing))};
            ray.origin += ray.direction;
            auto hitOpt = world->raycast(ray,range,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));
            
            if(hitOpt) {
                auto hit = hitOpt.value();
                Terrain* terrain = dynamic_cast<Terrain*>(hit.actor);
                if(terrain != nullptr) {
                    terrain->terraformSphere(world,hit.point,radius,-amount,false);
                }
            }

        }

        string getTypeName() override {
            return "drill";
        }
};