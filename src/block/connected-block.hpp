#pragma once

#include "block.hpp"
#include "actor/construction.hpp"
#include "helper/block-helper.hpp"
#include "slope-block.hpp"


class ConnectedBlock : public Block {
    public:

    
        ConnectedBlock() : Block() {
            solid = true;
        }

        TextureID texture = 0;
        Block* slope = nullptr;

        int connectionType = 0; // :shrug: idk

        virtual void addToMesh(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,BlockStorage& storage) {
            BlockHelper::addConnectedBlock(construction,meshData,position,texture);
        }


        void onHammer(Construction* construction,vec3 hammerPosition,ivec3 position,BlockStorage& storage) override {

            ivec3 quadrant = glm::floor((glm::mod(hammerPosition+0.5f,1.0f)*3.0f))-1.0f;
                    
            int slopeOrientation = -1;
            if(quadrant.z == 1) {
                if(quadrant.x == -1 && quadrant.y == 0) {
                    slopeOrientation = 0;
                }
                if(quadrant.x == 0 && quadrant.y == -1) {
                    slopeOrientation = 1;
                }
                if(quadrant.x == 1 && quadrant.y == 0) {
                    slopeOrientation = 2;
                }
                if(quadrant.x == 0 && quadrant.y == 1) {
                    slopeOrientation = 3;
                }
            }
            if(quadrant.z == 0) {
                if(quadrant.x == -1 && quadrant.y == -1) {
                    slopeOrientation = 4;
                }
                if(quadrant.x == 1 && quadrant.y == -1) {
                    slopeOrientation = 5;
                }
                if(quadrant.x == 1 && quadrant.y == 1) {
                    slopeOrientation = 6;
                }
                if(quadrant.x == -1 && quadrant.y == 1) {
                    slopeOrientation = 7;
                }
            }
            if(quadrant.z == -1) {
                if(quadrant.x == -1 && quadrant.y == 0) {
                    slopeOrientation = 8;
                }
                if(quadrant.x == 0 && quadrant.y == -1) {
                    slopeOrientation = 9;
                }
                if(quadrant.x == 1 && quadrant.y == 0) {
                    slopeOrientation = 10;
                }
                if(quadrant.x == 0 && quadrant.y == 1) {
                    slopeOrientation = 11;
                }
            }

            auto blockData = construction->getBlockData(position);
            if(slopeOrientation != -1 && slope != nullptr) {

                BlockStorage storage;
                storage.setInt(0,slopeOrientation);
                construction->placeBlock(position,slope,storage,blockData.attached);
            }
        }

        string getTypeName() override {
            return "connected";
        }
};