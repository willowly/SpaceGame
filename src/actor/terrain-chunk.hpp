#include "helper/terrain-helper.hpp"
#include "graphics/mesh.hpp"
#include <shared_mutex>
#include <vector>
#include "glm/glm.hpp"
#include "item/item.hpp"
#include "SimplexNoise.h"
#include "terrain-structs.hpp"
#include <mutex>
#include "helper/clock.hpp"

#include <map>

#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

#include <physics/jolt-terrain-shape.hpp>
#include <physics/jolt-userdata.hpp>

#include "FastNoiseLite.h"


#define SURFACE_LVL 0.5

class TerrainChunk {

    ivec3 offset = {}; //offset in overall terrain cell-space

    std::vector<VoxelData> terrainData;
    MeshData<TerrainVertex> meshData;
    int meshState = -1;
    MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
    bool gpuMeshOutOfDate = false;
    bool physicsMeshOutOfDate = false;
    TerrainType terrainTypes[8];
    std::atomic<bool> meshOutOfDate;

    TerrainShape* physicsShape = nullptr;

    unsigned int id = 0;
    int size = 30;
    float cellSize = 0.5f;

    unsigned int seed = 0;

    std::shared_timed_mutex mtx;
    std::atomic<bool> readyToRender = false;

    JPH::Body* body = nullptr;

    ActorUserData physicsUserData;

    // TerrainChunk(const TerrainChunk& chunk) = delete;
    // TerrainChunk& operator=(const TerrainChunk& chunk) = delete;

    int getPointIndex(int x,int y,int z) {
        x = std::clamp(x,0,size-1);
        y = std::clamp(y,0,size-1);
        z = std::clamp(z,0,size-1);
        return x + y * size + z * size * size;
    }

    float getPoint(int x,int y,int z) {

        std::shared_lock lock(mtx);
        return getPointUnsafe(x,y,z);
    }

    float getPointUnsafe(int x,int y,int z) {

        if(isPlaceHolder) {
            return 0;
        }
        if(x < 0) {
            x = 0;
        }
        if(x >= size) {
            if(posX != nullptr) {
                return posX->getPoint(x - size,y,z);
            } else {
                x = size-1;
            }
        }

        if(y < 0) {
            y = 0;
        }
        if(y >= size) {
            if(posY != nullptr) {
                return posY->getPoint(x,y - size,z);
            } else {
                y = size-1;
            }
        }

        if(z < 0) {
            z = 0;
        }
        if(z >= size) {
            if(posZ != nullptr) {
                return posZ->getPoint(x,y,z - size);
            } else {
                z = size-1;
            }
        }
        // if(x < 0 || x >= chunkSize) return 0;
        // if(y < 0 || y >= chunkSize) return 0;
        // if(z < 0 || z >= chunkSize) return 0;
        return terrainData.at(x + y * size + z * size * size).amount;
    }

    bool getPointInsideUnsafe(int x,int y,int z) {
        
        return getPointUnsafe(x,y,z) > SURFACE_LVL;
    }

    vec3 getCellLocalPos(ivec3 cell) {
        vec3 cellPos = (vec3)cell + vec3(0.5) + (vec3)offset;
        cellPos = cellPos*cellSize;
        return cellPos;
    }

    ivec3 localToCellPos(vec3 local) {
        local /= cellSize;
        local -= offset;
        return (ivec3)glm::floor(local);
    }

    vec3 getTerrainLocalOffset() {
        return ((vec3)offset)*cellSize;
    }

    TerrainChunk* posX = nullptr;
    TerrainChunk* posZ = nullptr;
    TerrainChunk* posY = nullptr;



    public:

        std::atomic<bool> isPlaceHolder = false;

        static TerrainChunk makePlaceHolder() {
            return TerrainChunk();
        }

        bool isReadyToRender() {
            return readyToRender;
        }

        int vertexCount() {
            return meshData.vertices.size();
        }

        // 
        bool isAvailable() {
            std::shared_lock lock(mtx,std::defer_lock);
            if(!lock.try_lock()) {
                return false;
            }
            if(cellSize == 0) {
                return true;
            }
            return false;
        }

        TerrainChunk() : isPlaceHolder(true) {
        }

        TerrainChunk(ivec3 offset,int chunkSize,float cellSize,unsigned int id,unsigned int seed) : offset(offset), size(chunkSize), cellSize(cellSize), id(id), seed(seed) {

            for (auto& buffer : meshBuffer)
            {
                buffer.buffer = VK_NULL_HANDLE;
            }
            
        }

        // turns a placeholder into a chunk that can be generated and stuff
        void create(ivec3 offset,int chunkSize,float cellSize,unsigned int id,unsigned int seed) {
            std::unique_lock lock(mtx,std::defer_lock);
            
            if(!lock.try_lock_for(std::chrono::seconds(3))) {
                throw std::runtime_error("timeout");
            }

            isPlaceHolder = false;
            this->offset = offset;
            this->size = chunkSize;
            this->cellSize = cellSize;
            this->id = id;
            this->seed = seed;
        }

        unsigned int getID() {
            return id;
        }

        void generateData(GenerationSettings settings,int layer) {

            std::unique_lock lock(mtx);
            terrainTypes[0] = settings.stoneType;
            terrainTypes[1] = settings.oreType;
            terrainData.resize(size*size*size);


            Clock clock;

            FastNoiseLite noise;
            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            noise.SetFractalType(FastNoiseLite::FractalType_FBm);
            noise.SetFractalOctaves(settings.noiseOctaves);
            noise.SetFractalGain(settings.noiseGain);
            noise.SetFractalLacunarity(settings.noiseLacunarity);
            noise.SetSeed(seed);

            //const SimplexNoise noise;

            // Gather noise data

            int i = 0;
            for (int z = 0; z < size; z++)
            {
                
                //std::cout << "generating terrain " << percent << "%" << std::endl;
                for (int y = 0; y < size; y++)
                {

                    for (int x = 0; x < size; x++)
                    {
                        vec3 samplePos = vec3(x,y,z);
                        samplePos += offset;
                        samplePos *= cellSize;
                        vec3 noiseSamplePos = samplePos * settings.noiseScale;
                        float noiseValue = noise.GetNoise(noiseSamplePos.x,noiseSamplePos.y,noiseSamplePos.z);
                        //float noiseValue = noise.fractal(5,noiseSamplePos.x,noiseSamplePos.y,noiseSamplePos.z);


                        float distance = glm::length(samplePos);
                        terrainData[i].amount = (settings.radius-distance) + (noiseValue * settings.noiseFactor);
                        i++;
                    }
                }
            }

            
            generateOre(1,5,0.7f,offset);
            //std::cout << "generation time:" << clock.getTime() << std::endl;
            meshOutOfDate = true;
            // generateOre(2,60,0.4,offset,chunk);
        }

        void generateOre(int id,float scale,float surfaceLevel,vec3 offset) {
            FastNoiseLite noise;
            noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
            noise.SetFractalType(FastNoiseLite::FractalType_FBm);
            noise.SetFractalOctaves(2);
            noise.SetSeed(seed);
            int i = 0;
            for (int z = 0; z < size; z++)
            {
                int percent = ((float)z/size)*100;
                //std::cout << "generating ore " << percent << "%" << std::endl;
                for (int y = 0; y < size; y++)
                {
                    for (int x = 0; x < size; x++)
                    {
                        vec3 samplePos = vec3(x,y,z);
                        samplePos += offset;
                        samplePos *= scale;
                        samplePos *= cellSize;
                        samplePos += vec3(size*id);
                        float oreNoise = noise.GetNoise(samplePos.x,samplePos.y,samplePos.z);
                        if(oreNoise > surfaceLevel) {
                            terrainData[i].type = id;
                        }
                        i++;
                    }
                }
            }
        }

        //position is in terrain space
        void terraformSphere(vec3 pos,float radius,float change,TerraformResults& results) {


            std::unique_lock lock(mtx);
            
            auto posCellSpace = localToCellPos(pos);
            assert(cellSize > 0);
            auto radiusCellSpace = radius/cellSize;
            // std::cout << radiusCellSpace << std::endl;
            // std::cout << posCellSpace.z-radiusCellSpace << std::endl;
            // std::cout << posCellSpace.z+radiusCellSpace << std::endl;
            for (int z = floor(std::max(0.0f,posCellSpace.z-radiusCellSpace)); z <= ceil(std::min((float)size,posCellSpace.z+radiusCellSpace)); z++)
            {
                for (int y = floor(std::max(0.0f,posCellSpace.y-radiusCellSpace)); y <= ceil(std::min((float)size,posCellSpace.y+radiusCellSpace)); y++)
                {
                    for (int x = floor(std::max(0.0f,posCellSpace.x-radiusCellSpace)); x <= ceil(std::min((float)size,posCellSpace.x+radiusCellSpace)); x++)
                    {
                        int i = getPointIndex(x,y,z);
                        
                        vec3 cellPos = getCellLocalPos(vec3(x,y,z));
                        float dist = glm::distance(cellPos,pos);
                        float influence = (radius-dist)/radius;
                        
                        if(influence > 0) {
                            float old = terrainData[i].amount;
                            terrainData[i].amount += change * influence;
                            terrainData[i].amount = std::min(std::max(terrainData[i].amount,0.0f),1.0f);
                            int terrainTypeID = terrainData[i].type;
                            
                            if(old > SURFACE_LVL && terrainData[i].amount < SURFACE_LVL) {
                                results.addItem(ItemStack(terrainTypes[terrainTypeID].item,1));
                            }
                            meshOutOfDate = true;
                        }
                        i++;
                    }
                }
            }
        }

        //adds a physics body if needed. terrain pointer is stored in userdata
        void updatePhysics(World* world,Actor* parent,vec3 position) {

            std::unique_lock lock(mtx,std::defer_lock);
            
            if(!lock.try_lock()) {
                return; // we dont update physics today
            }

            if(body == nullptr) {

                if(meshData.vertices.size() == 0 || meshData.indices.size() == 0) {
                    physicsMeshOutOfDate = true;
                    return;
                }

                TerrainShapeSettings settings(meshData,terrainData,size*cellSize);

                JPH::Shape::ShapeResult result;

                physicsShape = new TerrainShape(settings,result);

                // we can do something please
                JPH::BodyCreationSettings bodySettings(physicsShape, Physics::toJoltVec(position), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

                physicsUserData.actor = parent;
                physicsUserData.component = id;
                bodySettings.mUserData = ActorUserData::encode(&physicsUserData);

                if(result.HasError()) {
                    throw std::runtime_error((string)result.GetError());
                }

                

                body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
                world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);

                physicsMeshOutOfDate = false;

            } else {
                if(physicsShape != nullptr && physicsMeshOutOfDate) {
                    physicsShape->UpdateMesh(meshData,terrainData); //copy over the data :3
                    physicsMeshOutOfDate = false;
                }
             }

            
            

            
        }

        string getDebugInfo() {
            string r;
            r += std::format("\tchunk {}\n",id);
            r += std::format("\t\tv {}\n",meshData.vertices.size());
            r += std::format("\t\ti {}\n",meshData.indices.size());
            r += std::format("\t\to {}\n",StringHelper::toString(offset));
            return r;
        }
        

        void addRenderables(Vulkan* vulkan,float dt,vec3 position,Material material) {
            if(!readyToRender) return;
            std::shared_lock lock(mtx,std::defer_lock);
            
            if(lock.try_lock()) {
                if(meshState == -1) {
                    if(meshData.vertices.size() == 0 || meshData.indices.size() == 0) return;
                    meshBuffer[0] = vulkan->createMeshBuffers(meshData.vertices,meshData.indices);
                    //meshBuffer[1] = vulkan->createMeshBuffers(meshData.vertices,meshData.indices);
                    gpuMeshOutOfDate = false;
                    meshState = 0;
                } else {
                    if(gpuMeshOutOfDate) {
                        meshState++;
                        if(meshState >= FRAMES_IN_FLIGHT) meshState = 0;
                        vulkan->updateMeshBuffer(meshBuffer[meshState],meshData.vertices,meshData.indices);
                        gpuMeshOutOfDate = false;
                    }
                }
                lock.unlock();
            }
            if(meshState != -1) {
                vulkan->addMesh(meshBuffer[meshState],material,glm::translate(glm::mat4(1.0f),position+((vec3)offset*cellSize)));
            }
        }

        void drawDebug(vec3 position) {

            std::unique_lock lock(mtx,std::defer_lock);

            if(!lock.try_lock()) {
                return;
            }

            auto totalOffset = position+((vec3)offset*cellSize);
            for (size_t i = 0; i+2 < meshData.indices.size(); i += 3)
            {
                size_t indexA = meshData.indices[i];
                size_t indexB = meshData.indices[i+1];
                size_t indexC = meshData.indices[i+2];
                //std::cout << indexA << " " << shape2->terrain->vertices.size() << " " << &shape2->terrain->vertices << std::endl;
                vec3 a = meshData.vertices[indexA].pos + totalOffset;
                vec3 b = meshData.vertices[indexB].pos + totalOffset;
                vec3 c = meshData.vertices[indexC].pos + totalOffset;
                Debug::drawLine(a,b);
                Debug::drawLine(b,c);
                Debug::drawLine(c,a);
                if(i > 1000) {
                    return;
                }
            }

        }

        // force
        void generateMesh(bool force = false) {

            if(!meshOutOfDate && !force) {
                return;
            }
            
            std::unique_lock lock(mtx);
            
            float clock = (float)glfwGetTime();
            meshData.vertices.clear();
            meshData.indices.clear();


            int i = 0;
            for (int z = 0; z < size; z++)
            {
                int percent = ((float)z/size)*100;
                //std::cout << "generating mesh " << percent << "%" << std::endl;
                for (int y = 0; y < size; y++)
                {
                    
                    for (int x = 0; x < size; x++)
                    {
                        int config = 0;
                        if(getPointInsideUnsafe(x,y,z)) config |= 1;
                        if(getPointInsideUnsafe(x+1,y,z)) config |= 2;
                        if(getPointInsideUnsafe(x+1,y,z+1)) config |= 4;
                        if(getPointInsideUnsafe(x,y,z+1)) config |= 8;
                        if(getPointInsideUnsafe(x,y+1,z)) config |= 16;
                        if(getPointInsideUnsafe(x+1,y+1,z)) config |= 32;
                        if(getPointInsideUnsafe(x+1,y+1,z+1)) config |= 64;
                        if(getPointInsideUnsafe(x,y+1,z+1)) config |= 128;
                        addCellUnsafe(config,vec3(x,y,z));
                        i++;
                    }
                }
            }

            

            smoothNormals();
            
            //std::cout << "chunk ready to render " << std::endl;
            readyToRender = true;
            meshOutOfDate = false;
            gpuMeshOutOfDate = true;
            physicsMeshOutOfDate = true;
        }

        void generateEdges() {
            std::scoped_lock lock(mtx);

            
        }

        void smoothNormals() {

            //consolodate normals
            std::vector<TerrainVertex> newVertices;
            unordered_map<vec3,std::vector<TerrainVertex*>> vertexMap;
            for(auto& i : meshData.indices) {
                auto& vertex = meshData.vertices[i];
                if(!vertexMap.contains(vertex.pos)) {
                    vertexMap[vertex.pos] = std::vector<TerrainVertex*>();
                }
                vertexMap[vertex.pos].push_back(&vertex); //we can't resize the vector un
            }

            // smooth normals
            for (auto p : vertexMap)
            {
                auto& verticesAtPoint = p.second;
                vec3 normal = vec3(0);
                //average the normals at that point
                for(auto v : verticesAtPoint) {
                    normal += v->normal;
                }
                normal /= verticesAtPoint.size();
                // reapply the averaged normal
                for(auto v : verticesAtPoint) {
                    v->normal = normal;
                }
            }
        }

        void addCellUnsafe(int config,vec3 cellPos) {
            if(config < 0) {
                //std::cout << "out of range " << config << std::endl;
                return;
            }
            if(config > 255) {
                //std::cout << "out of range " << config << std::endl;
                return;
            }

            const int* tris = TerrainHelper::triTable[config];
            int i = 0;
            int startIndex = meshData.indices.size();
            int cellIndex = getPointIndex(cellPos.x,cellPos.y,cellPos.z);
            terrainData[cellIndex].verticesStart = startIndex;
            int face[3];
            ivec4 textureIDVec = ivec4(0);
            while(i < 100) //break out if theres a problem lol
            {
                int edge = tris[i];
                if(edge == -1) break;
                vec3 aPos = getEdgePos(edge,0)+cellPos;
                vec3 bPos = getEdgePos(edge,1)+cellPos;
                float a = getPointUnsafe((int)aPos.x,(int)aPos.y,(int)aPos.z);
                float b = getPointUnsafe((int)bPos.x,(int)bPos.y,(int)bPos.z);
                
                
                
                float t = (SURFACE_LVL - a)/(b-a);
                t = std::min(std::max(t,0.0f),1.0f);
                if(t < 0.0001f) { //hopefully this smooshes the unfavorable degen tris
                    t = 0.0f;
                }
                if(t > 9.9999f) {
                    t = 1.0f;
                }

                
                int closestCellIndex;
                if(a > b) {
                    closestCellIndex = getPointIndex((int)aPos.x,(int)aPos.y,(int)aPos.z);
                } else {
                    closestCellIndex = getPointIndex((int)bPos.x,(int)bPos.y,(int)bPos.z);
                }
                int vertIndex = i % 3;

                TextureID texture = terrainTypes[terrainData[closestCellIndex].type].texture;

                // std::cout << vertIndex << std::endl;
                
                auto vertex = TerrainVertex((getEdgePos(edge,t) + cellPos)*cellSize);
                
                textureIDVec[vertIndex] = texture;
                vertex.oreBlend[vertIndex] = 1;

                #ifdef TERRAIN_DEBUG_CHECKER
                    ivec3 chunkPos = offset/size;
                    textureIDVec[vertIndex] = (chunkPos.x+chunkPos.y+chunkPos.z) % 2 ? 1 : 0;
                    vertex.oreBlend[vertIndex] = 1;
                #endif
                
                meshData.vertices.push_back(vertex);
                face[vertIndex] = meshData.vertices.size()-1;
                //std::cout << (meshData.vertices.size()-1) << std::endl;
                assert(meshData.vertices.size()-1 < std::numeric_limits<uint16_t>().max());
                assert(meshData.vertices.size()-1 >= 0);
                meshData.indices.push_back(meshData.vertices.size()-1);
                if(vertIndex == 2) {

                    //remove "degenerate" triangles
                    if(isDegenerate(face)) {
                        meshData.vertices.erase(meshData.vertices.end()-3,meshData.vertices.end());
                        meshData.indices.erase(meshData.indices.end()-3,meshData.indices.end());
                    } else {
                    
                        int aIndex = face[0];
                        int bIndex = face[1];
                        int cIndex = face[2];
                        TerrainVertex& aVert = meshData.vertices[aIndex];
                        TerrainVertex& bVert = meshData.vertices[bIndex];
                        TerrainVertex& cVert = meshData.vertices[cIndex];
                        vec3 normal = MathHelper::normalFromPlanePoints(aVert.pos,bVert.pos,cVert.pos);
                        aVert.normal = normal;
                        aVert.textureID = textureIDVec;
                        bVert.normal = normal;
                        bVert.textureID = textureIDVec;
                        cVert.normal = normal;
                        cVert.textureID = textureIDVec;
                    }
                }
                i++;
            }
            terrainData[cellIndex].verticesEnd = meshData.indices.size();
            //meshData.indices.clear();
            
            
        }

        bool isDegenerate(int face[3]) {
            vec3 a = meshData.vertices[face[0]].pos;
            vec3 b = meshData.vertices[face[1]].pos;
            vec3 c = meshData.vertices[face[2]].pos;

            return glm::length2(glm::cross((b - a),(c - a))) <= 1.0e-11f;
        }

        void connectPosX(TerrainChunk* chunk) {

            std::unique_lock lock(mtx);

            if(posX != chunk) {
                posX = chunk;
                meshOutOfDate = true;
                //generateMesh();
            }
        }

        void connectPosZ(TerrainChunk* chunk) {

            std::unique_lock lock(mtx);

            if(posZ != chunk) {
                posZ = chunk;
                meshOutOfDate = true;
                //generateMesh();
            }
        }

        void connectPosY(TerrainChunk* chunk) {

            std::unique_lock lock(mtx);
            
            if(posY != chunk) {
                posY = chunk;
                meshOutOfDate = true;
                //generateMesh();
            }
        }

        vec3 getEdgePos(int index,float t) {
            switch(index) {
                case 0:
                    return vec3(t,0,0);
                case 1:
                    return vec3(1,0,t);
                case 2:
                    return vec3(t,0,1);
                case 3:
                    return vec3(0,0,t);
                case 4:
                    return vec3(t,1,0);
                case 5:
                    return vec3(1,1,t);
                case 6:
                    return vec3(t,1,1);
                case 7:
                    return vec3(0,1,t);
                case 8:
                    return vec3(0,t,0);
                case 9:
                    return vec3(1,t,0);
                case 10:
                    return vec3(1,t,1);
                case 11:
                    return vec3(0,t,1);

            }
            return vec3(0,0,0);
        }

public:
    
};