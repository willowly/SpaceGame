
#pragma once
#include "graphics/mesh.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"
#include "item/item.hpp"
#include "helper/location-key.hpp"
#include "terrain-chunk.hpp"
#include "terrain/terrain-structs.hpp"
#include <math.h>
#include <thread>
#include <chrono>
#include "helper/random.hpp"

using std::unique_ptr, std::string;
using glm::vec3, glm::ivec4,glm::vec4;

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include "physics/jolt-conversions.hpp"

struct ChunkAddress {
    int layer = 0;
    ivec3 pos;
};


class Terrain : public Actor {

    

    Terrain() : Actor() {
        chunkLayers =  std::vector<std::map<LocationKey,TerrainChunk>>(LODlayers);
    }

    GenerationSettings settings;
    std::vector<std::map<LocationKey,TerrainChunk>> chunkLayers;
    Material material = Material::none;

    std::shared_mutex chunksMtx;

    std::atomic<int> lockType = 0;

    int chunkSize = 30; //without LOD, all chunks are the same size
    float cellSize = 0.5f;

    int currentLODlayer = -1;
    static const int LODlayers = 3;
    static const int LODscaleFactor = 4;

    std::shared_mutex loadedLayersMtx;
    std::array<bool,LODlayers> loadedLayers{};
    int nextLODlayer = 0; // the one that should be loaded next

    float LODdistance = 300;
    float LODdistanceFactor = 3; // so the distance between each is a factor of 2

    unsigned int seed = 0;

    std::atomic<unsigned int> nextChunkId;
    
    public:
    
    // DEBUG
    int selectedChunk = 0;
    // DEBUG
    
    void setCurrentLOD(int layer) {

        
        if(layer < 0 || layer >= LODlayers) return;

        if(currentLODlayer == -1) currentLODlayer = layer; //if we dont have a current layer, dont even check
        
        std::shared_lock lock(loadedLayersMtx,std::defer_lock);

        if(!lock.try_lock()) return;
        if(!loadedLayers[layer]) return;
        currentLODlayer = layer;
    }

    void setNextLOD(int layer) {
        if(layer < 0 || layer >= LODlayers) return;
        nextLODlayer = layer;
    }

    void changeLOD(int change) {
        setCurrentLOD(currentLODlayer + change);
    }

    // adds a chunk to the terrain. Able to be called on loader thread
    void addChunk(ChunkAddress address) {
        ZoneScoped
        int layer = address.layer;
        ivec3 pos = address.pos;

        assert(layer >= 0 && layer < LODlayers);
        
        LocationKey key(pos);
        ivec3 offset = pos*chunkSize;

        float newCellSize = cellSize * powf(LODscaleFactor,layer);
        
        TerrainChunk* chunk = nullptr;

        {
            std::unique_lock lock(chunksMtx);
            lockType = 101;
            auto& chunks = chunkLayers.at(address.layer);
            bool contains = chunks.contains(key);
            if(contains && !chunks.at(key).isPlaceHolder) {
                std::cout << std::this_thread::get_id() << "chunk not available" << std::endl;
                return;
            }
            chunks.at(key).create(offset,chunkSize,newCellSize,nextChunkId,seed);
            //std::cout << std::this_thread::get_id() << "adding chunk " << std::endl;
            chunk = &chunks.at(key);
        }

        assert(chunk != nullptr);

        nextChunkId++;

        chunk->generateData(settings,address.layer);
        //chunk.generateMesh();
        
        connect(*chunk,address); // this will end up generating the mesh for us :)

        if(!chunk->isReadyToRender()) {
            throw std::runtime_error("should be ready to render");
        }
        
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

        std::shared_lock lock(chunksMtx);
        lockType = 100;
        // for(auto& pair : chunks) {
        //     if(pair.second.getID() == component) {
        //         r += pair.second.getDebugInfo();
        //     }
        // }
        return r;
    }

    void addPlaceholder(ChunkAddress address) {
        std::unique_lock lock(chunksMtx);
        auto& chunks = chunkLayers.at(address.layer);
        lockType = 505;
        chunks.emplace(std::piecewise_construct,std::make_tuple(address.pos),std::make_tuple());
    }

    std::optional<ChunkAddress> getNextChunkToload(vec3 cameraPosition) {
        ZoneScoped
        assert(currentLODlayer >= 0 && currentLODlayer < LODlayers);

        //std::cout << "terrain at " << StringHelper::toString(position) << std::endl;
        vec3 cameraPositionRelative = inverseTransformPoint(cameraPosition);
        vec3 cameraPositionChunk = glm::floor(cameraPositionRelative/((float)chunkSize*cellSize));


        {
            //std::cout << "inside chunk " << StringHelper::toString(cameraPositionChunk);
            std::shared_lock lock(chunksMtx);
            auto& chunks = chunkLayers[0];
            lockType = 5000;
            auto key = LocationKey(cameraPositionChunk);
            // if(!chunks.contains(key)) {
            //     //std::cout << " chunk doesn't exist" << std::endl;
            // } else {
            //     if(chunks.at(key).isPlaceHolder) {
            //         //std::cout << " chunk is a placeholder" << std::endl;
            //     } else {
            //         std::cout << chunks.at(key).vertexCount() << "verts" << std::endl;
            //     }

            // }
        }

        int layerToLoad = currentLODlayer;
        {
            std::shared_lock lock(loadedLayersMtx);
            if(loadedLayers.at(layerToLoad)) {
                layerToLoad = nextLODlayer;
            }
        
        }
        
        int size = getChunkGridSize(layerToLoad);
        bool chunkFound = false;
        float closestChunkDist = std::numeric_limits<float>::max();
        ivec3 closestChunkPos = {};
        auto& chunks = chunkLayers[layerToLoad];
        // generate one extra
        for (int z = -size; z <= size; z++)
        {
            for (int y = -size; y <= size; y++)
            {
                for (int x = -size; x <= size; x++)
                {
                    ivec3 chunkPos = ivec3(x,y,z);
                    LocationKey key(chunkPos);
                    std::shared_lock lock(chunksMtx);
                    lockType = 1;
                    if(!chunks.contains(key)) {
                        float dist = glm::length(vec3(x,y,z) - cameraPositionChunk);
                        if(dist < closestChunkDist) {
                            closestChunkPos = chunkPos;
                            closestChunkDist = dist;
                            chunkFound = true;
                        }
                        
                    }
                   
                }
            }
        }

        if(!chunkFound) {
            std::lock_guard lock(loadedLayersMtx);
            loadedLayers.at(layerToLoad) = true;
            return std::nullopt;
        }

        return ChunkAddress{layerToLoad,closestChunkPos};
        
    }
    // how big the overall grid of chunks is
    // range of valid coordinates is (-size,size). 0 means 1 single chunk
    int getChunkGridSize(int layer) {
        int chunkSize = getChunkWorldSize(layer);
        float extent = std::ceilf(settings.radius/chunkSize);
        return extent;
    }

    int getChunkWorldSize(int layer) {
        return chunkSize * cellSize * pow(LODscaleFactor,layer);
    }

    void prePhysics(World* world) override {

        // ZoneScopedN("prePhysicsTerrain")
        Clock clock;

        auto chunks = getChunksLocked(0);
        auto time = clock.getTime();
        if(time > 0.01f) {
            Debug::warn(" main thread blocked for " + std::to_string((int)(time*1000)) + "ms");
        }

        for(auto& pair : chunks) {
            ivec3 pos = pair.first;
            auto chunk = pair.second;
            vec3 offset = pos*chunkSize;
            chunk->updatePhysics(world,this,position + (vec3)offset*cellSize);
        }
    }

    std::vector<std::pair<ivec3,TerrainChunk*>> getChunksLocked(int layer) {
        std::vector<std::pair<ivec3,TerrainChunk*>> array;
        std::shared_lock lock(chunksMtx);
        lockType = 2;
        auto& chunkLayer = chunkLayers[layer];
        array.reserve(chunkLayer.size());
        for(auto& pair : chunkLayer) {

            ivec3 pos = pair.first.asVec3();
            TerrainChunk* chunk = &pair.second;
            array.push_back(std::pair<ivec3,TerrainChunk*>(pos,chunk));
        }
        return array;
    }

    
    void connect(TerrainChunk& chunk,ChunkAddress address) {

        std::vector<TerrainChunk*> chunksSurrounding; 

        int layer = address.layer;
        ivec3 pos = address.pos;

        chunksSurrounding.reserve(8);

        {
            std::shared_lock lock(chunksMtx);
            lockType = 301;
            auto& chunks = chunkLayers[layer];
            LocationKey keyPosX(pos+ivec3(1,0,0));
            if(chunks.contains(keyPosX) && !chunks.at(keyPosX).isPlaceHolder) {
                chunk.connectPosX(&chunks.at(keyPosX));
            }
            LocationKey keyNegX(pos+ivec3(-1,0,0));
            if(chunks.contains(keyNegX) && !chunks.at(keyNegX).isPlaceHolder) {
                chunks.at(keyNegX).connectPosX(&chunk);
            }

            LocationKey keyPosY(pos+ivec3(0,1,0));
            if(chunks.contains(keyPosY) && !chunks.at(keyPosY).isPlaceHolder) {
                chunk.connectPosY(&chunks.at(keyPosY));
            }
            LocationKey keyNegY(pos+ivec3(0,-1,0));
            if(chunks.contains(keyNegY) && !chunks.at(keyNegY).isPlaceHolder) {
                chunks.at(keyNegY).connectPosY(&chunk);
            }

            LocationKey keyPosZ(pos+ivec3(0,0,1));
            if(chunks.contains(keyPosZ) && !chunks.at(keyPosZ).isPlaceHolder) {
                chunk.connectPosZ(&chunks.at(keyPosZ));
            }
            LocationKey keyNegZ(pos+ivec3(0,0,-1));
            if(chunks.contains(keyNegZ) && !chunks.at(keyNegZ).isPlaceHolder) {
                chunks.at(keyNegZ).connectPosZ(&chunk);
            }
            lockType = 302;
            for (int z = 0; z <= 1; z++)
            {
                for (int y = 0; y <= 1; y++)
                {
                    for (int x = 0; x <= 1; x++)
                    {
                        ivec3 chunkPos = pos - ivec3(x,y,z);
                        LocationKey key(chunkPos);
                        if(chunks.contains(key) && !chunks.at(key).isPlaceHolder) {
                            chunksSurrounding.push_back(&chunks.at(key));
                        }
                    }
                }
            }
            lockType = 303;
        }

        // to leave things locked for less time
        for(auto chunkRegen : chunksSurrounding) {
            chunkRegen->generateMesh(true);
        }
        
    }

    void spawn(World* world) override {

        updateLOD(world);

    }

    void step(World* world,float dt) override {

        updateLOD(world);

    }

    void updateLOD(World* world) {
        float distance = glm::length(world->getCamera().position - position);

        setCurrentLODBasedOnDistance(distance);
    }
    

    void setCurrentLODBasedOnDistance(float distance) {
        for (int i = 0; i < LODlayers; i++)
        {
            if(distance < LODdistance * pow(LODdistanceFactor,i)) {
                setCurrentLOD(i);
                if(currentLODlayer == 0) {
                    nextLODlayer = 1;
                } else if (currentLODlayer == LODlayers - 1) {
                    nextLODlayer = currentLODlayer - 1;
                } else {
                    if (distance < LODdistance * pow(LODdistanceFactor,i-0.5f)) {
                        nextLODlayer = currentLODlayer - 1;
                    } else {
                        nextLODlayer = currentLODlayer + 1;
                    }
                }
                return;
            }
        }
        setCurrentLOD(LODlayers - 1); //fallback option
    }

    

    void terraformSphere(World* world,vec3 pos,float radius,float change) {
        
        TerraformResults results;
        {
            std::lock_guard lock(chunksMtx);
            lockType = 4;
            vec3 localPosition = inverseTransformPoint(pos);
            auto& chunks = chunkLayers[0];
            for(auto& pair : chunks) {
                auto& chunk = pair.second;
                chunk.terraformSphere(localPosition,radius,change,results);
            }
            for(auto& pair : chunks) {
                auto& chunk = pair.second;
                chunk.generateMesh(); //only generates if it needs an update
            }
        }


        for (auto stack : results.items)
        {
            world->spawn(ItemActor::makeInstance(stack,pos,Random::rotation()));
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

    void addRenderables(Vulkan* vulkan,float dt) override {

        ZoneScopedN("Terrain::addRenderables");
        Clock clock;
        std::shared_lock lock(chunksMtx);
        auto time = clock.getTime();
        if(time > 0.01f) {
            Debug::warn(" render thread blocked for " + std::to_string((int)(time*1000)) + "ms lock type:" + std::to_string(lockType));
        }
        auto& chunks = chunkLayers[currentLODlayer];

        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.addRenderables(vulkan,dt,position,material);
        }
        //std::cout << "render time: " << (float)glfwGetTime() - clock << std::endl;
    }

    // ivec3 getCellAtWorldPos(vec3 pos) {
    //     return glm::floor(inverseTransformPoint(pos)/cellSize);
    // }


    
    
    //need to manually regenerate the mesh (in case things want to do multiple)

    static std::unique_ptr<Terrain> makeInstance(Material material,GenerationSettings settings,int seed,vec3 position = vec3(0)) {
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