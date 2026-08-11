#pragma once
#include "actor/construction.hpp"
#include "block/block.hpp"

namespace BlockHelper {

    BlockFacing getFacingFromVector(vec3 v);

    quat getRotationFromFacing(BlockFacing blockFacing);

    quat getRotationFromFacing(BlockFacing blockFacing,int rotationIndex);

    int getRotationIndexFromVector(vec2 direction);

    BlockFacing rotateFacingByFacing(BlockFacing a, BlockFacing b);

    void addBlockFace(Construction* construction,MeshData<ConstructionVertex>& meshData,vec3 position,quat rotation,TextureID textureID,Rect texRect = Rect::unitSquare,bool sloped = false);
    

    void addSingleBlock(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,TextureID texture,quat rotation = glm::identity<quat>());


    void addMesh(MeshData<ConstructionVertex>& meshData,ivec3 position,quat rotation,const MeshData<Vertex>& blockMeshData,TextureID texture);

    void addMesh(MeshData<ConstructionVertex>& meshData,ivec3 position,BlockFacing facing,Mesh<Vertex>* blockMesh,TextureID texture);

    void addMesh(MeshData<ConstructionVertex>& meshData,ivec3 position,BlockFacing facing,int rotationIndex,Mesh<Vertex>* blockMesh,TextureID texture);


    ivec3 rotateByFacing(ivec3 vec,BlockFacing facing);

    void addConnectedBlockFace(Construction* construction,MeshData<ConstructionVertex>& meshData,vec3 position,quat rotation,TextureID textureID) ;

    void addConnectedBlock(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,TextureID texture);

    void addSlopedBlock(Construction* construction,MeshData<ConstructionVertex>& meshData,ivec3 position,int orientation,TextureID texture);
}