#pragma once
#include "actor/construction.hpp"
#include "block/block.hpp"

namespace BlockHelper {

    BlockFacing getFacingFromVector(vec3 v) {
            v = glm::normalize(v);
            if(v.z == 1) {
                return BlockFacing::FORWARD;
            }
            if(v.z == -1) {
                return BlockFacing::BACKWARD;
            }
            if(v.y == 1) {
                return BlockFacing::UP;
            }
            if(v.y == -1) {
                return BlockFacing::DOWN;
            }
            if(v.x == 1) {
                return BlockFacing::RIGHT;
            }
            if(v.x == -1) {
                return BlockFacing::LEFT;
            }
            return BlockFacing::FORWARD;
        }

        quat getRotationFromFacing(BlockFacing blockFacing) {
            switch(blockFacing) {
                case BlockFacing::FORWARD:
                    return glm::angleAxis(glm::radians(0.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::BACKWARD:
                    return glm::angleAxis(glm::radians(180.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::UP:
                    return glm::angleAxis(glm::radians(-90.0f),vec3(1.0f,0,0));
                    break;
                case BlockFacing::DOWN:
                    return glm::angleAxis(glm::radians(90.0f),vec3(1.0f,0,0));
                    break;
                case BlockFacing::RIGHT:
                    return glm::angleAxis(glm::radians(90.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::LEFT:
                    return glm::angleAxis(glm::radians(-90.0f),vec3(0,1.0f,0));
                    break;
            }
        }

        BlockFacing rotateFacingByFacing(BlockFacing a, BlockFacing b) {
            return getFacingFromVector(vec3(0,0,1) * getRotationFromFacing(a) * getRotationFromFacing(b));
        }

        void addBlockFace(Construction* construction,MeshData<ConstructionVertex>& meshData,vec3 position,quat rotation,TextureID textureID,Rect texRect = Rect::unitSquare) {


            if(construction == nullptr) {
                Debug::warn("construction is null during mesh build");
                return;
            }


            int indexOffset = meshData.vertices.size();
            auto faceVerts = vector<ConstructionVertex> {

                // the texcoords aren't how you'd expect but if you think about it makes sense (I hope) idk theres weirdness about texture sampling
                ConstructionVertex(Vertex(vec3(0.5,0.5,0.5),vec3(0.0,0.0,1.0),texRect.bottomRight()),textureID),
                ConstructionVertex(Vertex(vec3(0.5,-0.5,0.5),vec3(0.0,0.0,1.0),texRect.topRight()),textureID),
                ConstructionVertex(Vertex(vec3(-0.5,0.5,0.5),vec3(0.0,0.0,1.0),texRect.bottomLeft()),textureID),
                ConstructionVertex(Vertex(vec3(-0.5,-0.5,0.5),vec3(0.0,0.0,1.0),texRect.topLeft()),textureID)
            };
            for(auto vertex : faceVerts) {
                vertex.pos = rotation * vertex.pos;
                vertex.pos += position;
                vertex.normal = rotation * vertex.normal;
                meshData.vertices.push_back(vertex); 
            }
            auto faceIndices = vector<uint16_t> {
                0,1,2,1,2,3
            };
            for(auto index : faceIndices) {
                meshData.indices.push_back(index + indexOffset); 
            }
        }
        

        void addSingleBlock(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,quat rotation,TextureID texture) {

            if(construction == nullptr) {
                Debug::warn("construction is null during mesh build");
                return;
            }
            
            if(!construction->solidInDirection(position,ivec3(0,0,1)))  addBlockFace(construction,meshData,position,rotation,                                     texture);
            if(!construction->solidInDirection(position,ivec3(0,0,-1))) addBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0,180,0))), texture);
            if(!construction->solidInDirection(position,ivec3(1,0,0)))  addBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0, 90,0))) ,texture);
            if(!construction->solidInDirection(position,ivec3(-1,0,0))) addBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0,-90,0))) ,texture);
            if(!construction->solidInDirection(position,ivec3(0,1,0)))  addBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(-90,0,0))) ,texture); //this is flipped from how id expect :shrug: 
            if(!construction->solidInDirection(position,ivec3(0,-1,0))) addBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3( 90,0,0))) ,texture);
        }


        void addMesh(MeshData<ConstructionVertex>& meshData,ivec3 position,quat rotation,const MeshData<Vertex>& blockMeshData,TextureID texture) {
            int indexOffset = meshData.vertices.size();
            for(auto vertex : blockMeshData.vertices) {
                vertex.pos = rotation * vertex.pos;
                vertex.pos += position;
                vertex.normal = rotation * vertex.normal;
                meshData.vertices.push_back(ConstructionVertex(vertex,texture)); 
            }
            for(auto index : blockMeshData.indices) {
                meshData.indices.push_back(index + indexOffset); 
            }
        }

        void addMesh(MeshData<ConstructionVertex>& meshData,ivec3 position,BlockFacing facing,Mesh<Vertex>* blockMesh,TextureID texture) {
            if(blockMesh == nullptr) {
                Debug::warn("tried to render block with no mesh");
            }
            quat rotation = BlockHelper::getRotationFromFacing(facing);
            BlockHelper::addMesh(meshData,position,rotation,blockMesh->meshData,texture);
        }

        ivec3 rotateByFacing(ivec3 vec,BlockFacing facing) {
            return glm::round(getRotationFromFacing(facing) * (vec3)vec); //kinda crude but it gets the job done
        }
        


        void addConnectedBlockFace(Construction* construction,MeshData<ConstructionVertex>& meshData,vec3 position,quat rotation,TextureID textureID) {
            ivec2 gridSize(8,8);

            if(construction == nullptr) {
                Debug::warn("construction is null during mesh build");
                return;
            }

            

            char connectivity = 0;
            connectivity |= construction->solidInDirection(position,glm::round(rotation * vec3(0,1,0))) << 0;
            connectivity |= construction->solidInDirection(position,glm::round(rotation * vec3(1,0,0))) << 1;
            connectivity |= construction->solidInDirection(position,glm::round(rotation * vec3(0,-1,0))) << 2;
            connectivity |= construction->solidInDirection(position,glm::round(rotation * vec3(-1,0,0))) << 3;

            bool topRight    = construction->solidInDirection(position,glm::round(rotation * vec3(1,1,0)));
            bool topLeft     = construction->solidInDirection(position,glm::round(rotation * vec3(-1,1,0)));
            bool bottomRight = construction->solidInDirection(position,glm::round(rotation * vec3(1,-1,0)));
            bool bottomLeft  = construction->solidInDirection(position,glm::round(rotation * vec3(-1,-1,0)));

            int id = 0;

            const int TOP_BIT = 0b0001;
            const int BOTTOM_BIT = 0b0100;
            const int RIGHT_BIT = 0b0010;
            const int LEFT_BIT = 0b1000;
            // vertical 1x3
            switch(connectivity) {
                case TOP_BIT:
                    id = 8;
                    break;
                case (TOP_BIT | BOTTOM_BIT):
                    id = 16;
                    break;
                case BOTTOM_BIT:
                    id = 24;
                    break;
                // horizontal 1x3
                case RIGHT_BIT:
                    id = 1;
                    break;
                case (RIGHT_BIT | LEFT_BIT):
                    id = 2;
                    break;
                case LEFT_BIT:
                    id = 3;
                    break;
                // corners
                case (TOP_BIT | RIGHT_BIT):
                    id = 9;
                    if(!topRight) {
                        id = 20;
                    } 
                    break;
                case (TOP_BIT | LEFT_BIT):
                    id = 11;
                    if(!topLeft) {
                        id = 21;
                    } 
                    break;
                case (BOTTOM_BIT | RIGHT_BIT):
                    id = 25;
                    if(!bottomRight) {
                        id = 28;
                    } 
                    break;
                case (BOTTOM_BIT | LEFT_BIT):
                    id = 27;
                    if(!bottomLeft) {
                        id = 29;
                    } 
                    break;

                //edges
                case (TOP_BIT | RIGHT_BIT | LEFT_BIT):
                    id = 10;
                    if(!topRight) {
                        id = 46;
                    } 
                    if(!topLeft) {
                        id = 38;
                    } 
                    if(!topLeft && !topRight) {
                        id = 35;
                    } 
                    break;
                case (TOP_BIT | BOTTOM_BIT | RIGHT_BIT):
                    id = 17;
                    if(!topRight) {
                        id = 42;
                    } 
                    if(!bottomRight) {
                        id = 40;
                    } 
                    if(!topRight && !bottomRight) {
                        id = 32;
                    } 
                    break;
                case (TOP_BIT | BOTTOM_BIT | LEFT_BIT):
                    id = 19;
                    if(!topLeft) {
                        id = 43;    
                    } 
                    if(!bottomLeft) {
                        id = 41;
                    } 
                    if(!topLeft && !bottomLeft) {
                        id = 33;
                    } 
                    break;
                case (BOTTOM_BIT | RIGHT_BIT | LEFT_BIT):
                    id = 26;
                    if(!bottomLeft) {
                        id = 44;    
                    } 
                    if(!bottomRight) {
                        id = 45;
                    } 
                    if(!bottomLeft && !bottomRight) {
                        id = 34;
                    }
                    break;
                //middle
                case (TOP_BIT | BOTTOM_BIT | RIGHT_BIT | LEFT_BIT):
                    id = 18;
                    
                    // one corner
                    if(!topRight) {
                        id = 4;    
                    }
                    if(!topLeft) {
                        id = 5;    
                    }
                    if(!bottomRight) {
                        id = 12;    
                    }
                    if(!bottomLeft) {
                        id = 13;    
                    }
                    // two corners adjacent
                    if(!topRight && !topLeft) { //top
                        id = 7;    
                    }
                    if(!topRight && !bottomRight) { //right
                        id = 6;    
                    }
                    if(!bottomRight && !bottomLeft) { //bottom
                        id = 14;    
                    }
                    if(!topLeft && !bottomLeft) { //left
                        id = 15;    
                    }
                    // two corners diagonal
                    if(!topRight && !bottomLeft) { // /forward
                        id = 47;    
                    }
                    if(!topLeft && !bottomRight) { // \back
                        id = 39;    
                    }
                    // three corners
                    if(!topLeft && !bottomRight && !bottomLeft) { //top right missing
                        id = 22;
                    }
                    if(!topRight && !bottomRight && !bottomLeft) { //top left missing
                        id = 23;    
                    }
                    if(!topLeft  && !topRight && !bottomLeft) { //bottom right missing
                        id = 30;    
                    }
                    if(!topLeft  && !topRight && !bottomRight) { //bottom left missing
                        id = 31;    
                    }
                    // all four
                    if(!topLeft && !topRight && !bottomRight && !bottomLeft) {
                        id = 48;    
                    }
                    break;
            }

            ivec2 gridPos = ivec2((id % gridSize.x),id/gridSize.y); //see below comment about backwardsness

            Rect rect((vec2)gridPos/(vec2)gridSize,1.0f/(vec2)gridSize); 

            addBlockFace(construction,meshData,position,rotation,textureID,rect);
        }

        void addConnectedBlock(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,TextureID texture) {

            if(construction == nullptr) {
                Debug::warn("construction is null during mesh build");
                return;
            }

            quat rotation = glm::identity<quat>(); //just here in case I change my mind
            
            if(!construction->solidInDirection(position,ivec3(0,0,1)))  addConnectedBlockFace(construction,meshData,position,rotation,                                     texture);
            if(!construction->solidInDirection(position,ivec3(0,0,-1))) addConnectedBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0,180,0))), texture);
            if(!construction->solidInDirection(position,ivec3(1,0,0)))  addConnectedBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0, 90,0))) ,texture);
            if(!construction->solidInDirection(position,ivec3(-1,0,0))) addConnectedBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(0,-90,0))) ,texture);
            if(!construction->solidInDirection(position,ivec3(0,1,0)))  addConnectedBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3(-90,0,0))) ,texture); //this is flipped from how id expect :shrug: 
            if(!construction->solidInDirection(position,ivec3(0,-1,0))) addConnectedBlockFace(construction,meshData,position,rotation * quat(glm::radians(vec3( 90,0,0))) ,texture);
        }
}