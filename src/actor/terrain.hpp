
#pragma once
#include "graphics/mesh.hpp"
#include "actor/actor.hpp"
#include "helper/string-helper.hpp"
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

#include "actor/item-actor.hpp"

#include "persistance/actor/data-terrain.hpp"

struct ChunkAddress {
    int layer = 0;
    ivec3 pos = {};
    ChunkAddress() {};
    ChunkAddress(int layer,ivec3 pos) : layer(layer),pos(pos) {}
};


class Terrain : public Actor {

    

    Terrain() : Actor() {
        
    }

    GravityWell gravityWell;

    TerrainSettings settings;
    std::vector<std::map<LocationKey,TerrainChunk>> chunkLayers;
    Material material = Material::none;

    std::shared_mutex chunksMtx;

    bool renderChildren = false;

    std::atomic<int> lockType = 0;


    std::shared_mutex loadedLayersMtx;
    //std::array<bool,LODlayers> loadedLayers{};
    int nextLODlayer = 0; // the one that should be loaded next

    unsigned int seed = 0;

    std::vector<ChunkAddress> chunksToRegenerate{};

    std::atomic<unsigned int> nextChunkId;
    
    public:
    
    // DEBUG
    int selectedChunk = 0;
    // DEBUG
    


    // adds a chunk to the terrain. Able to be called on loader thread
    void addChunk(ChunkAddress address) {
        ZoneScoped
        int layer = address.layer;
        ivec3 pos = address.pos;

        assert(layer >= 0 && layer < settings.LODlayers);
        
        LocationKey key(pos);
        ivec3 offset = pos*settings.chunkSize;

        float newCellSize = settings.baseCellSize * powf(TerrainChunk::LODscaleFactor,layer);
        
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
            chunks.at(key).create(offset,settings.chunkSize,newCellSize,nextChunkId,seed);
            //std::cout << std::this_thread::get_id() << "adding chunk " << std::endl;
            chunk = &chunks.at(key);
        }

        assert(chunk != nullptr);

        nextChunkId++;

        chunk->generateData(settings.generationSettings,address.layer);
        //chunk.generateMesh();
        
        connect(*chunk,address); // this will end up generating the mesh for us :)

        if(!chunk->isReadyToRender()) {
            throw std::runtime_error("should be ready to render");
        }
        
    }

    ivec3 worldToChunkPos(vec3 position) {
        return glm::floor(position/getChunkWorldSizeBase());
    }
    ivec3 worldToChunkPosRounded(vec3 position) {
        return glm::round(position/getChunkWorldSizeBase());
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

    void testNextChunkToLoad(ChunkAddress address,ChunkAddress& closest,float& closestDistance,vec3 cameraPosition,bool& chunkFound,bool allowNextLayer) {
        auto& chunks = chunkLayers[address.layer];
        float size = getChunkWorldSize(address.layer);
        vec3 center = TerrainChunk::getWorldCenter(position,address.pos,size);
        vec3 closestPoint = MathHelper::getClosestPointOnBox(cameraPosition,center,vec3(size/2.0f));
        float dist = glm::length(closestPoint - cameraPosition);
        if(!chunks.contains(address.pos)) {
            if(dist < closestDistance) {
                closestDistance = dist;
                closest = address;
                chunkFound = true;
            }
        } else {
            auto& chunk = chunks[address.pos];
            if(allowNextLayer && address.layer != 0 && dist < 2 * settings.LODdistance * pow(2,address.layer)) {
                for (int z = 0; z <= 1; z++)
                {
                    for (int y = 0; y <= 1; y++)
                    {
                        for (int x = 0; x <= 1; x++)
                        {
                            auto key = (address.pos * TerrainChunk::LODscaleFactor) + ivec3(x,y,z);
                            testNextChunkToLoad(ChunkAddress(address.layer-1,key),closest,closestDistance,cameraPosition,chunkFound,chunk.allChildrenReady());
                        }
                    }
                }
            }
        }
    }

    std::optional<ChunkAddress> getNextChunkToload(vec3 cameraPosition) {
        ZoneScoped

        //std::cout << "terrain at " << StringHelper::toString(position) << std::endl;
        vec3 cameraPositionRelative = inverseTransformPoint(cameraPosition);
        vec3 cameraPositionChunk = glm::floor(cameraPositionRelative/getChunkWorldSizeBase());


        
            //std::cout << "inside chunk " << StringHelper::toString(cameraPositionChunk);
        //std::shared_lock lock(chunksMtx);
            // if(!chunks.contains(key)) {
            //     //std::cout << " chunk doesn't exist" << std::endl;
            // } else {
            //     if(chunks.at(key).isPlaceHolder) {
            //         //std::cout << " chunk is a placeholder" << std::endl;
            //     } else {
            //         std::cout << chunks.at(key).vertexCount() << "verts" << std::endl;
            //     }

            // }
        

        int topLayer = chunkLayers.size()-1;
        
        int size = getChunkGridSize(topLayer);
        bool chunkFound = false;
        float closestChunkDist = std::numeric_limits<float>::max();
        ChunkAddress closestChunkAddress = {};
        auto& chunks = chunkLayers[topLayer];
        // generate one extra
        for (int z = -size; z < size; z++)
        {
            for (int y = -size; y < size; y++)
            {
                for (int x = -size; x < size; x++)
                {
                    ivec3 chunkPos = ivec3(x,y,z);
                    LocationKey key(chunkPos);
                    std::shared_lock lock(chunksMtx);
                    lockType = 1;
                    testNextChunkToLoad(ChunkAddress(topLayer,chunkPos),closestChunkAddress,closestChunkDist,cameraPosition,chunkFound,true);
                }
            }
        }

        if(!chunkFound) {
            //std::lock_guard lock(loadedLayersMtx);
            //loadedLayers.at(topLayer) = true;
            return std::nullopt;
        }

        return closestChunkAddress;
        
    }
    // how big the overall grid of chunks is
    // range of valid coordinates is (-size,size). 0 means 1 single chunk
    int getChunkGridSize(int layer) {
        int chunkSize = getChunkWorldSize(layer);
        float extent = std::ceilf(1.2f*settings.generationSettings.radius/chunkSize);
        return extent;
    }

    float getChunkWorldSize(int layer) {
        return getChunkWorldSizeBase() * pow(TerrainChunk::LODscaleFactor,layer);
    }

    float getChunkWorldSizeBase() {
        return (float)settings.chunkSize*settings.baseCellSize;
    }

    void prePhysics(World* world) override {

        // ZoneScopedN("prePhysicsTerrain")
        Clock clock;

        auto chunks = getChunksLocked(0);
        auto time = clock.getTime();
        if(time > 0.1f) {
            Debug::warn(" main thread blocked for " + std::to_string((int)(time*1000)) + "ms");
        }

        for(auto& pair : chunks) {
            ivec3 pos = pair.first;
            auto chunk = pair.second;
            vec3 offset = pos*settings.chunkSize;
            chunk->updatePhysics(world,this,position + (vec3)offset*settings.baseCellSize);
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
            // if(layer > 0) { //if not smallest layer
            //     auto& smallerLayer = chunkLayers[layer-1];
            //     for (int x = 0;x < TerrainChunk::LODscaleFactor;x++) {
            //         for (int y = 0;y < TerrainChunk::LODscaleFactor;y++) {
            //             for (int z = 0;z < TerrainChunk::LODscaleFactor;z++) {
            //                 ivec3 childPos = ivec3(x,y,z);
            //                 ivec3 smallerPos = pos *= TerrainChunk::LODscaleFactor;
            //                 smallerPos += childPos;
            //                 if(smallerLayer.contains(smallerPos) && !smallerLayer.at(smallerPos).isPlaceHolder) {
            //                     chunk.setChild(&smallerLayer.at(smallerPos),childPos);
            //                 }
            //             }
            //         }
            //     }
            // }
            if(layer+1 < settings.LODlayers) { // if not largest layer
                auto& largerLayer = chunkLayers[layer+1];
                vec3 largerPosUnrounded = (vec3)pos / (float)TerrainChunk::LODscaleFactor;

                ivec3 largerPos = glm::floor(largerPosUnrounded);
                if(largerLayer.contains(largerPos) && !largerLayer.at(largerPos).isPlaceHolder) {
                    ivec3 childPos = MathHelper::mod(pos,TerrainChunk::LODscaleFactor);
                    //std::cout << "childPos: " << StringHelper::toString(childPos) << std::endl;
                    largerLayer.at(largerPos).setChild(&chunk, childPos);
                }
            }
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

        world->addGravityWell(&gravityWell);

    }

    void step(World* world,float dt) override {

        updateLOD(world);

        regenerateChunkQueue();

    }

    void destroy(World* world) override {
        std::shared_lock lock(chunksMtx);
        lockType = 77;
        for(auto& chunkLayer : chunkLayers) {
            for(auto& pair : chunkLayer) {
                auto& chunk = pair.second;
                chunk.destroy(world);
            }
        }
        world->removeGravityWell(&gravityWell);
    }

    void updateLOD(World* world) {
        std::shared_lock lock(chunksMtx);
        //float distance = glm::length(world->getCamera().position - position);
        auto& chunks = chunkLayers.back();

        for(auto& pair : chunks) {
            auto& chunk = pair.second;
            chunk.updateLOD(position,world->getCamera().position,chunkLayers.size() - 1,settings.LODdistance);
        }

        //setCurrentLODBasedOnDistance(distance);
    }


    void regenerateChunkQueue() {
        if(chunksToRegenerate.size() == 0) return;
        std::lock_guard lock(chunksMtx);
        lockType = 4;
        for(int i = 0;i < chunksToRegenerate.size();i++) {
            auto address = chunksToRegenerate[i];
            chunkLayers[address.layer].at(address.pos).generateMesh();
        }
        chunksToRegenerate.clear();
    }
    
    void queueChunkRegeneration(ChunkAddress address) {
        chunksToRegenerate.push_back(address);
    }

    TerraformResults terraformSphere(World* world,vec3 pos,float radius,float change,bool spawnItems = true) {
        
        TerraformResults results;
        {
            std::lock_guard lock(chunksMtx);
            lockType = 4;
            vec3 localPosition = inverseTransformPoint(pos);
            auto& chunks = chunkLayers[0];
            for(auto& pair : chunks) {
                auto& chunk = pair.second;
                if(chunk.terraformSphere(localPosition,radius,change,results)) {
                    queueChunkRegeneration({0,pair.first.asVec3()});
                }
            }
            // for(auto& pair : chunks) {
            //     auto& chunk = pair.second;
            //     chunk.generateMesh(); //only generates if it needs an update
            // }
            
        }

        if(spawnItems) {
            for (auto stack : results.items)
            {
                world->spawn(ItemActor::makeInstance(stack,pos,Random::rotation()));
            }
        }

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

    void addRenderables(Vulkan* vulkan,float dt,float interpolation) override {

        ZoneScopedN("Terrain::addRenderables");
        Clock clock;
        std::shared_lock lock(chunksMtx);
        auto time = clock.getTime();
        if(time > 0.1f) {
            Debug::warn(" render thread blocked for " + std::to_string((int)(time*1000)) + "ms lock type:" + std::to_string(lockType));
        }

        // start at lowest level
        auto& chunks = chunkLayers.back();

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

    static std::unique_ptr<Terrain> makeInstance(Material material,TerrainSettings settings,int seed,vec3 position = vec3(0)) {
        auto ptr = new Terrain();
        ptr->material = material;
        ptr->settings = settings; 
        ptr->position = position;
        ptr->gravityWell = GravityWell(position,settings.gravity,settings.generationSettings.radius);
        ptr->chunkLayers = std::vector<std::map<LocationKey,TerrainChunk>>(settings.LODlayers);
        return std::unique_ptr<Terrain>(ptr);
    }

    string getTypeName() override {
        return "terrain";
    }

    string getActorDataType() override {
        return getTypeName();
    }

    virtual std::vector<std::uint8_t> createSaveBuffer() {
        auto data = save();
        auto buf = cista::serialize(data);
        return buf;
    }

    data_Terrain save() {
        data_Terrain data;
        data.actor = Actor::save();
        data.seed = seed;
        data.terrainSettings = settings.name;
        for(auto& chunkLayer : chunkLayers) {
            data.chunkLayers.push_back({});
            auto& dataChunkLayer = data.chunkLayers.back();
            for(auto& pair : chunkLayer) {
                auto& chunk = pair.second;
                dataChunkLayer.chunks.push_back(chunk.save(pair.first.asVec3()));
            }
        }

        return data;
    }


    void load(data_Terrain data,DataLoader& loader) {
        Actor::load(data.actor);
    }

    static std::unique_ptr<Actor> makeInstanceFromSave(data_Terrain& data,Material material,DataLoader& loader) {
        auto settings = loader.getTerrainSettings((string)data.terrainSettings);
        if(settings == nullptr) {
            Debug::warn("terrain settings missing: " + (string)data.terrainSettings);
            return nullptr;
        }
        auto actor = makeInstance(material,*settings,data.seed,vec3(0.0f));
        actor->load(data,loader);
        std::cout << "LOADING TERRAIN ACTOR" << std::endl;

        return actor;
    }

};