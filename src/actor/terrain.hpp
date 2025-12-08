
#pragma once
#include "graphics/mesh.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"
#include "item/item.hpp"
#include "helper/location-key.hpp"
#include "terrain-chunk.hpp"
#include "terrain-structs.hpp"
#include <math.h>

using std::unique_ptr;
using glm::vec3, glm::ivec4,glm::vec4;


class Terrain : public Actor {

    

    Terrain() : Actor() {

    }

    GenerationSettings settings;
    std::map<LocationKey,TerrainChunk> chunks;
    Material material = Material::none;

    int chunkSize = 30; //without LOD, all chunks are the same size
    float cellSize = 0.5f;

    public:

        


    void addChunk(ivec3 pos) {
        LocationKey key(pos);
        chunks.emplace(key,TerrainChunk(pos,chunkSize,cellSize));
        auto& chunk = chunks.at(key);
        chunk.generateData(settings);
        chunk.generateMesh();
    }

    void loadChunks(vec3 position,int distance) {
        ivec3 pos = glm::floor(position/((float)chunkSize*cellSize));
        pos.y = 0;
        for (int z = -distance; z <= distance; z++)
        {
            for (int x = -distance; x <= distance; x++)
            {
                LocationKey key(pos + ivec3(x,0,z));
                if(!chunks.contains(key)) {
                    std::cout << key.x << "," << key.y << "," << key.z << std::endl;
                    addChunk(pos + ivec3(x,0,z));
                }
            }
        }
        
    }
    


    // void generateOre(int id,float scale,float surfaceLevel,vec3 offset,Chunk& chunk) {
    //     const SimplexNoise simplex;

    //     auto& terrainData = chunk.terrainData;

    //     int i = 0;
    //     for (int z = 0; z < chunkSize; z++)
    //     {
    //         int percent = ((float)z/chunkSize)*100;
    //         //std::cout << "generating ore " << percent << "%" << std::endl;
    //         for (int y = 0; y < chunkSize; y++)
    //         {
    //             for (int x = 0; x < chunkSize; x++)
    //             {
    //                 vec3 samplePos = vec3(x,y,z);
    //                 samplePos += offset;
    //                 samplePos /= scale;
    //                 samplePos += vec3(chunkSize*id);
    //                 float oreNoise = simplex.fractal(5,samplePos.x,samplePos.y,samplePos.z);
    //                 if(oreNoise > surfaceLevel) {
    //                     terrainData[i].type = id;
    //                 }
    //                 i++;
    //             }
    //         }
    //     }
    // }

    virtual void addRenderables(Vulkan* vulkan,float dt) {
        
        for(auto& pair : chunks) {
            auto chunk = pair.second;
            chunk.addRenderables(vulkan,dt,position,material);
        }
        //std::cout << "render time: " << (float)glfwGetTime() - clock << std::endl;
    }

    // ivec3 getCellAtWorldPos(vec3 pos) {
    //     return glm::floor(inverseTransformPoint(pos)/cellSize);
    // }

    virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {

        return std::nullopt;

        // std::optional<RaycastHit> result = std::nullopt;
        // Ray localRay = Ray(inverseTransformPoint(ray.origin),inverseTransformDirection(ray.direction));
        // for(size_t i = 0;i+2 < indices.size();i += 3)
        // {
        //     vec3 a = vertices[indices[i]].pos;
        //     vec3 b = vertices[indices[i+1]].pos;
        //     vec3 c = vertices[indices[i+2]].pos;

        //     auto hitopt = Physics::intersectRayTriangle(a,b,c,localRay);
        //     if(hitopt) {
                
        //         auto hit = hitopt.value();
        //         if(hit.distance <= dist) {
        //             hit.point = transformPoint(hit.point);
        //             hit.normal = transformDirection(hit.normal);
        //             result = hit;
        //             dist = hit.distance; //distance stays the same when transformed
        //         }
        //     }
                
        // }
        // //std::cout << "raycast time: " << (float)glfwGetTime() - clock << std::endl;
        // return result;
    }

    virtual void collideBasic(Actor* actor,float height,float radius) {
        return;
        // vec3 localActorPosition = inverseTransformPoint(actor->position);
        // vec3 localActorPositionTop = inverseTransformPoint(actor->transformPoint(vec3(0,height,0)));
        // Debug::drawLine(actor->position,actor->transformPoint(vec3(0,height,0)));
        // auto posCellSpace = getCellAtWorldPos(actor->position);
        // auto radiusCellSpace = (radius+height)/cellSize;
        // for (int z = std::max(0,(int)floor(posCellSpace.z-radiusCellSpace)); z <= std::min(size-1,(int)ceil(posCellSpace.z+radiusCellSpace)); z++)
        // {
        //     for (int y = std::max(0,(int)floor(posCellSpace.y-radiusCellSpace)); y <= std::min(size-1,(int)ceil(posCellSpace.y+radiusCellSpace)); y++)
        //     {
        //         for (int x = std::max(0,(int)floor(posCellSpace.x-radiusCellSpace)); x <= std::min(size-1,(int)ceil(posCellSpace.x+radiusCellSpace)); x++)
        //         {
        //             auto voxel = terrainData[getPointIndex(x,y,z)];
        //             for (int i = voxel.verticesStart;i < voxel.verticesEnd;i += 3)
        //             {
                        
        //                 vec3 a = vertices[indices[i]].pos;
        //                 vec3 b = vertices[indices[i+1]].pos;
        //                 vec3 c = vertices[indices[i+2]].pos;
        //                 // Debug::drawLine(transformPoint(a),transformPoint(b),Color::white);
        //                 // Debug::drawLine(transformPoint(b),transformPoint(c),Color::white);
        //                 // Debug::drawLine(transformPoint(c),transformPoint(a),Color::white);

        //                 // this should probably be wrapped in some collision shape of the actor
        //                 auto contact_opt = Physics::intersectCapsuleTri(localActorPosition,localActorPositionTop,radius,a,b,c);
        //                 if(contact_opt) {
                            
        //                     auto contact = contact_opt.value();

        //                     Debug::drawRay(contact.point,contact.normal*contact.penetration);
        //                     Physics::resolveBasic(localActorPosition,contact);
        //                     Physics::resolveBasic(localActorPositionTop,contact); // i mean... somethings gotta be wrong here lol
                            
        //                 }
                            
        //             } 
        //         }
        //     }
        // }
        // actor->position = transformPoint(localActorPosition);
    }
    
    //need to manually regenerate the mesh (in case things want to do multiple)

    static std::unique_ptr<Terrain> makeInstance(Material material,GenerationSettings settings,vec3 position = vec3(0)) {
        auto ptr = new Terrain();
        ptr->material = material;
        ptr->settings = settings;
        return std::unique_ptr<Terrain>(ptr);
    }


    // void drawCellsOnRay(Ray ray,float dist) {

        
    //     //assert(abs(glm::length(ray.direction) - 1) > 0.001);
        
    //     Debug::drawRay(ray.origin,ray.direction*10.0f,Color::white);
    //     Ray cellSpaceRay = Ray(transformPoint(ray.origin),transformDirection(ray.direction));
    //     cellSpaceRay.origin = cellSpaceRay.origin/cellSize;
    //     float cellDist = dist/cellSize;
    //     float yxSlope = cellSpaceRay.direction.y/abs(cellSpaceRay.direction.x);
    //     float zxSlope = cellSpaceRay.direction.z/abs(cellSpaceRay.direction.x);
    //     std::cout << cellDist*cellSpaceRay.direction.x << std::endl;
    //     int xDir = ray.direction.x > 0 ? 1 : -1;
    //     float xOffset = MathHelper::fromFloor(cellSpaceRay.origin.x*xDir);
    //     for(int i = 1;((i-1)*cellSize) <= (abs(cellSpaceRay.direction.x)*dist+xOffset);i++) {

            
            
    //         ivec3 cellPos = ivec3(
    //                 xDir*floor(i+cellSpaceRay.origin.x)-1,
    //                 MathHelper::integerBelow(((i-xOffset)*yxSlope)+cellSpaceRay.origin.y),
    //                 MathHelper::integerBelow(((i-xOffset)*zxSlope)+cellSpaceRay.origin.z)
    //         );
    //         Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::green);
    //     }

    //     float xySlope = cellSpaceRay.direction.x/cellSpaceRay.direction.y;
    //     float zySlope = cellSpaceRay.direction.z/cellSpaceRay.direction.y;
    //     for(int i = 1;i <= ceil(cellDist*cellSpaceRay.direction.y)+1;i++) {

    //         float offset = MathHelper::fromFloor(cellSpaceRay.origin.y);

    //         ivec3 cellPos = ivec3(
    //                 MathHelper::integerBelow(((i-offset)*xySlope)+cellSpaceRay.origin.x),
    //                 floor(i+cellSpaceRay.origin.y)-1,
    //                 MathHelper::integerBelow(((i-offset)*zySlope)+cellSpaceRay.origin.z)
    //         );
    //         Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::magenta);
    //     }

    //     float xzSlope = cellSpaceRay.direction.x/cellSpaceRay.direction.z;
    //     float yzSlope = cellSpaceRay.direction.y/cellSpaceRay.direction.z;
    //     for(int i = 1;i <= ceil(cellDist*cellSpaceRay.direction.z)+1;i++) {

    //         float offset = MathHelper::fromFloor(cellSpaceRay.origin.z);

    //         ivec3 cellPos = ivec3(
    //                 MathHelper::integerBelow(((i-offset)*xzSlope)+cellSpaceRay.origin.x),
    //                 MathHelper::integerBelow(((i-offset)*yzSlope)+cellSpaceRay.origin.y),
    //                 floor(i+cellSpaceRay.origin.z)-1
    //         );
    //         Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::red);
    //     }
    // }

};