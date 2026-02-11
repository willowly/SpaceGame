
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
#include <thread>
#include <chrono>

using std::unique_ptr, std::string;
using glm::vec3, glm::ivec4,glm::vec4;

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "physics/jolt-conversions.hpp"


class Terrain : public Actor {

    

    Terrain() : Actor() {

    }

    GenerationSettings settings;
    std::map<LocationKey,TerrainChunk> chunks;
    Material material = Material::none;

    std::mutex chunksMtx;

    int chunkSize = 30; //without LOD, all chunks are the same size
    float cellSize = 0.5f;

    int size = 5;

    unsigned int nextChunkId;
    
    public:
    
    // DEBUG
    int selectedChunk = 0;
    // DEBUG
    

    // adds a chunk to the terrain. Able to be called on loader thread
    void addChunk(World* world,ivec3 pos,Vulkan* vulkan) {
        
        LocationKey key(pos);
        ivec3 offset = pos*chunkSize;

        chunksMtx.lock();
        chunks.emplace(std::piecewise_construct,std::make_tuple(key),std::make_tuple(offset,chunkSize,cellSize,nextChunkId));
        auto& chunk = chunks.at(key);
        chunksMtx.unlock();

        nextChunkId++;

        chunk.generateData(settings);
        //chunk.generateMesh();
        
        connect(chunk,pos,vulkan); // this will end up generating the mesh for us :)

        
    }

    ivec3 worldToChunkPos(vec3 position) {
        return glm::floor(position/((float)chunkSize*cellSize));
    }
    ivec3 worldToChunkPosRounded(vec3 position) {
        return glm::round(position/((float)chunkSize*cellSize));
    }

    string getDebugInfo(int component = -1) {
        string r;
        r += std::format("terrain \n");

        std::scoped_lock lock(chunksMtx);
        for(auto& pair : chunks) {
            if(pair.second.getID() == component) {
                r += pair.second.getDebugInfo();
            }
        }
        return r;
    }

    void loadChunks(World* world,vec3 position,int distance,Vulkan* vulkan) {

        position = inverseTransformPoint(position);
        ivec3 pos = glm::floor(position/((float)chunkSize*cellSize));
        // generate one extra
        for (int z = -distance; z <= distance; z++)
        {
            for (int y = -distance; y <= distance; y++)
            {
                for (int x = -distance; x <= distance; x++)
                {
                    ivec3 chunkPos = pos + ivec3(x,y,z);
                    LocationKey key(chunkPos);
                    if(chunkPos.x < -size || chunkPos.x > size) {
                        continue;
                    }
                    if(chunkPos.z < -size || chunkPos.z > size) {
                        continue;
                    }
                    if(chunkPos.y < -size || chunkPos.y > size) {
                        continue;
                    }
                    if(!chunks.contains(key)) {
                        addChunk(world,chunkPos,vulkan);
                        return;
                    }
                   
                }
            }
        }
        // for (int z = -distance; z <= distance; z++)
        // {
        //     for (int y = -distance; y <= distance; y++)
        //     {
        //         for (int x = -distance; x <= distance; x++)
        //         {
        //             ivec3 chunkPos = pos + ivec3(x,y,z);
        //             LocationKey key(chunkPos);
        //             if(chunkPos.x < -size || chunkPos.x > size) {
        //                 continue;
        //             }
        //             if(chunkPos.z < -size || chunkPos.z > size) {
        //                 continue;
        //             }
        //             if(chunkPos.y < -size || chunkPos.y > size) {
        //                 continue;
        //             }
        //             if(chunks.contains(key)) {
        //                 chunks.at(key).updateMeshBuffers(vulkan);
        //             }
        //         }
        //     }
        // }
    }

    void prePhysics(World* world) override {

        // ZoneScopedN("prePhysicsTerrain")
        Clock clock;
        std::scoped_lock lock(chunksMtx);
        auto time = clock.getTime();
        if(time > 0.01f) {
            Debug::warn(" main thread blocked for " + std::to_string((int)(time*1000)) + "ms");
        }

        
        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            ivec3 pos = pair.first.asVec3();
            vec3 offset = pos*chunkSize;
            chunk.updatePhysics(world,this,position + (vec3)offset*cellSize);
        }
    }

    
    void connect(TerrainChunk& chunk,ivec3 pos,Vulkan* vulkan) {

        chunksMtx.lock();
        LocationKey keyPosX(pos+ivec3(1,0,0));
        if(chunks.contains(keyPosX)) {
            chunk.connectPosX(&chunks.at(keyPosX));
        }
        LocationKey keyNegX(pos+ivec3(-1,0,0));
        if(chunks.contains(keyNegX)) {
            chunks.at(keyNegX).connectPosX(&chunk);
        }

        LocationKey keyPosY(pos+ivec3(0,1,0));
        if(chunks.contains(keyPosY)) {
            chunk.connectPosY(&chunks.at(keyPosY));
        }
        LocationKey keyNegY(pos+ivec3(0,-1,0));
        if(chunks.contains(keyNegY)) {
            chunks.at(keyNegY).connectPosY(&chunk);
        }

        LocationKey keyPosZ(pos+ivec3(0,0,1));
        if(chunks.contains(keyPosZ)) {
            chunk.connectPosZ(&chunks.at(keyPosZ));
        }
        LocationKey keyNegZ(pos+ivec3(0,0,-1));
        if(chunks.contains(keyNegZ)) {
            chunks.at(keyNegZ).connectPosZ(&chunk);
        }

        std::vector<TerrainChunk*> chunksSurrounding; 

        chunksSurrounding.reserve(8);

        for (int z = 0; z <= 1; z++)
        {
            for (int y = 0; y <= 1; y++)
            {
                for (int x = 0; x <= 1; x++)
                {
                    ivec3 chunkPos = pos - ivec3(x,y,z);
                    LocationKey key(chunkPos);
                    if(chunks.contains(key)) {
                        chunksSurrounding.push_back(&chunks.at(key));
                        
                    }
                }
            }
        }
        chunksMtx.unlock();

        // to leave things locked for less time
        for(auto chunkRegen : chunksSurrounding) {
            chunkRegen->generateMesh(true);
        }
        
    }

    
    virtual void collideBasic(Actor* actor,float height,float radius) {
        vec3 localActorPosition = inverseTransformPoint(actor->getPosition());
        vec3 offset = actor->transformDirection(vec3(0,height,0));
        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.collideBasic(localActorPosition,offset,radius);
        }
        actor->setPosition(transformPoint(localActorPosition));
    }

    TerraformResults terraformSphere(vec3 pos,float radius,float change) {
            
        chunksMtx.lock();
        TerraformResults results;
        vec3 localPosition = inverseTransformPoint(pos);

        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.terraformSphere(localPosition,radius,change,results);
        }
        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.generateMesh(); //only generates if it needs an update
        }
        chunksMtx.unlock();

        return results;

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

    virtual void addRenderables(Vulkan* vulkan,float dt) override {

        Clock clock;
        std::scoped_lock lock(chunksMtx);
        auto time = clock.getTime();
        if(time > 0.01f) {
            Debug::warn(" render thread blocked for " + std::to_string((int)(time*1000)) + "ms");
        }

        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.addRenderables(vulkan,dt,position,material);
            if(chunk.getID() == selectedChunk) {
                chunk.drawDebug(position);
            }
        }
        //std::cout << "render time: " << (float)glfwGetTime() - clock << std::endl;
    }

    // ivec3 getCellAtWorldPos(vec3 pos) {
    //     return glm::floor(inverseTransformPoint(pos)/cellSize);
    // }

    virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {



        //ZoneScopedN("terrain raycast");

        std::unique_lock lock(chunksMtx,std::defer_lock);


        Ray localRay = Ray(inverseTransformPoint(ray.origin),inverseTransformDirection(ray.direction));

        std::optional<RaycastHit> result = std::nullopt;

        ivec3 pos = worldToChunkPosRounded(ray.origin);

        for (int z = 0; z <= 1; z++)
        {
            for (int y = 0; y <= 1; y++)
            {
                for (int x = 0; x <= 1; x++)
                {
                    ivec3 chunkPos = pos - ivec3(x,y,z);
                    LocationKey key(chunkPos);
                    if(chunks.contains(key)) {
                        auto hitOpt = chunks.at(key).raycast(localRay,dist);
                        if(hitOpt) {
                    
                            auto hit = hitOpt.value();
                            if(hit.distance <= dist) {
                                hit.point = transformPoint(hit.point);
                                hit.normal = transformDirection(hit.normal);
                                result = hit;
                                dist = hit.distance; //distance stays the same when transformed
                            }
                        }
                    }
                }
            }
        }

        
        return result;
    }

    
    
    //need to manually regenerate the mesh (in case things want to do multiple)

    static std::unique_ptr<Terrain> makeInstance(Material material,GenerationSettings settings,vec3 position = vec3(0)) {
        auto ptr = new Terrain();
        ptr->material = material;
        ptr->settings = settings; 
        ptr->position = position;
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