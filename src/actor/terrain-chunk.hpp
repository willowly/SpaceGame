#include "helper/terrain-helper.hpp"
#include "graphics/mesh.hpp"
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


#define SURFACE_LVL 0.5

class TerrainChunk {

    struct VoxelData {
        float amount = 0;
        int type;
        int verticesStart = 0;
        int verticesEnd = 0;
    };

    ivec3 offset; //offset in overall terrain cell-space

    std::vector<VoxelData> terrainData;
    MeshData<TerrainVertex> meshData;
    int meshState = -1;
    MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
    bool gpuMeshOutOfDate = false;
    bool physicsMeshOutOfDate = false;
    TerrainType terrainTypes[8];
    bool meshOutOfDate;

    TerrainShape* physicsShape = nullptr;;

    unsigned int id = 0;
    int size = 30;
    float cellSize = 0.5f;

    std::mutex mtx;
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
        return terrainData[x + y * size + z * size * size].amount;
    }

    bool getPointInside(int x,int y,int z) {
        
        return getPoint(x,y,z) > SURFACE_LVL;
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

        TerrainChunk(ivec3 offset,int chunkSize,float cellSize,unsigned int id) : offset(offset), size(chunkSize), cellSize(cellSize), id(id) {

            for (auto& buffer : meshBuffer)
            {
                buffer.buffer = VK_NULL_HANDLE;
            }
            
        }

        unsigned int getID() {
            return id;
        }

        void generateData(GenerationSettings settings) {

            std::scoped_lock lock(mtx);
            terrainTypes[0] = settings.stoneType;
            terrainTypes[1] = settings.oreType;
            terrainData.resize(size*size*size);

            const SimplexNoise simplex;

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
                        vec3 noiseSamplePos = samplePos / settings.noiseScale;
                        float noise = simplex.fractal(5,noiseSamplePos.x,noiseSamplePos.y,noiseSamplePos.z);
                        noise *= 0.5f;
                        noise += 0.5f;


                        float distance = glm::length(samplePos);
                        float radiusInfluence = (settings.radius-distance)/settings.radius;
                        terrainData[i].amount = noise * (radiusInfluence+0.5f);
                        i++;
                    }
                }
            }

            generateOre(1,30,0.5,offset);
            meshOutOfDate = true;
            // generateOre(2,60,0.4,offset,chunk);
        }

        void generateOre(int id,float scale,float surfaceLevel,vec3 offset) {
            const SimplexNoise simplex;


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
                        samplePos /= scale;
                        samplePos += vec3(size*id);
                        float oreNoise = simplex.fractal(5,samplePos.x,samplePos.y,samplePos.z);
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


            std::scoped_lock lock(mtx);
            
            auto posCellSpace = localToCellPos(pos);
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

                if(meshData.vertices.size() == 0 || meshData.indices.size() == 0) return;

                TerrainShapeSettings settings(meshData,size*cellSize);

                JPH::Shape::ShapeResult result;

                physicsShape = new TerrainShape(settings,result);

                // we can do something please
                JPH::BodyCreationSettings bodySettings(physicsShape, Physics::toJoltVec(position), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::NON_MOVING);

                physicsUserData.actor = parent;
                physicsUserData.component = id;
                bodySettings.mUserData = ActorUserData::encode(&physicsUserData);

                if(result.HasError()) {
                    std::cout << result.GetError() << std::endl;
                }

                

                body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
                world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);

                physicsMeshOutOfDate = false;

            } else {
                if(physicsShape != nullptr && physicsMeshOutOfDate) {
                    physicsShape->UpdateMesh(meshData); //copy over the data :3
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
            std::unique_lock lock(mtx,std::defer_lock);
            
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
            
            std::scoped_lock lock(mtx);
            
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
                        if(getPointInside(x,y,z)) config |= 1;
                        if(getPointInside(x+1,y,z)) config |= 2;
                        if(getPointInside(x+1,y,z+1)) config |= 4;
                        if(getPointInside(x,y,z+1)) config |= 8;
                        if(getPointInside(x,y+1,z)) config |= 16;
                        if(getPointInside(x+1,y+1,z)) config |= 32;
                        if(getPointInside(x+1,y+1,z+1)) config |= 64;
                        if(getPointInside(x,y+1,z+1)) config |= 128;
                        addCellNoLock(config,vec3(x,y,z));
                        i++;
                    }
                }
            }

            

            smoothNormals();
            
            
            readyToRender = true;
            gpuMeshOutOfDate = true;
            meshOutOfDate = false;
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

        void addCellNoLock(int config,vec3 cellPos) {
            if(config < 0) {
                std::cout << "out of range " << config << std::endl;
                return;
            }
            if(config > 255) {
                std::cout << "out of range " << config << std::endl;
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
                float a = getPoint((int)aPos.x,(int)aPos.y,(int)aPos.z);
                float b = getPoint((int)bPos.x,(int)bPos.y,(int)bPos.z);
                
                
                
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
                    }

                    vec3 normal = MathHelper::normalFromPlanePoints(meshData.vertices[face[0]].pos,meshData.vertices[face[1]].pos,meshData.vertices[face[2]].pos);
                    meshData.vertices[face[0]].normal = normal;
                    meshData.vertices[face[0]].textureID = textureIDVec;
                    meshData.vertices[face[1]].normal = normal;
                    meshData.vertices[face[1]].textureID = textureIDVec;
                    meshData.vertices[face[2]].normal = normal;
                    meshData.vertices[face[2]].textureID = textureIDVec;
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

        // need shape
        virtual void collideBasic(vec3& localActorPosition,vec3 topOffset,float radius) {
            if(!readyToRender) return;

            if(!mtx.try_lock()) {
                return;
            }

            auto posCellSpace = (localActorPosition/cellSize)-(vec3)offset;
            auto radiusCellSpace = (radius+glm::length(topOffset))/cellSize;

            vec3 localOffset = (vec3)offset*cellSize;
            for (int z = std::max(0,(int)floor(posCellSpace.z-radiusCellSpace)); z <= std::min(size-1,(int)ceil(posCellSpace.z+radiusCellSpace)); z++)
            {
                for (int y = std::max(0,(int)floor(posCellSpace.y-radiusCellSpace)); y <= std::min(size-1,(int)ceil(posCellSpace.y+radiusCellSpace)); y++)
                {
                    for (int x = std::max(0,(int)floor(posCellSpace.x-radiusCellSpace)); x <= std::min(size-1,(int)ceil(posCellSpace.x+radiusCellSpace)); x++)
                    {
                        auto voxel = terrainData[getPointIndex(x,y,z)];
                        for (int i = voxel.verticesStart;i < voxel.verticesEnd;i += 3)
                        {
                            
                            vec3 a = meshData.vertices[meshData.indices[i]].pos + localOffset;
                            vec3 b = meshData.vertices[meshData.indices[i+1]].pos + localOffset;
                            vec3 c = meshData.vertices[meshData.indices[i+2]].pos + localOffset;
                            // Debug::drawLine(transformPoint(a),transformPoint(b),Color::white);
                            // Debug::drawLine(transformPoint(b),transformPoint(c),Color::white);
                            // Debug::drawLine(transformPoint(c),transformPoint(a),Color::white);

                            // this should probably be wrapped in some collision shape of the actor
                            auto contact_opt = Physics::intersectCapsuleTri(localActorPosition,localActorPosition+topOffset,radius,a,b,c);
                            if(contact_opt) {
                                
                                auto contact = contact_opt.value();

                                Debug::drawRay(contact.point,contact.normal*contact.penetration);
                                Physics::resolveBasic(localActorPosition,contact);
                                
                            }
                                
                        } 
                    }
                }
            }
            mtx.unlock();
        }

        virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {

            if(!mtx.try_lock()) { //if a chunk isn't ready to load we can just assume its being loaded far away :)
                return std::nullopt;
            }
            std::optional<RaycastHit> result = std::nullopt;
            Ray localRay = Ray(ray.origin-getTerrainLocalOffset(),ray.direction);


            for(size_t i = 0;i+2 < meshData.indices.size();i += 3)
            {
                vec3 a = meshData.vertices[meshData.indices[i]].pos;
                vec3 b = meshData.vertices[meshData.indices[i+1]].pos;
                vec3 c = meshData.vertices[meshData.indices[i+2]].pos;

                auto hitopt = Physics::intersectRayTriangle(a,b,c,localRay);
                if(hitopt) {
                    
                    auto hit = hitopt.value();
                    if(hit.distance <= dist) {
                        hit.point = (hit.point) + getTerrainLocalOffset();
                        result = hit;
                        dist = hit.distance; //distance stays the same when transformed
                    }
                }
                    
            }
            mtx.unlock();

            return result;
        }

        void connectPosX(TerrainChunk* chunk) {

            std::scoped_lock lock(mtx);

            if(posX != chunk) {
                posX = chunk;
                meshOutOfDate = true;
                //generateMesh();
            }
        }

        void connectPosZ(TerrainChunk* chunk) {

            std::scoped_lock lock(mtx);

            if(posZ != chunk) {
                posZ = chunk;
                meshOutOfDate = true;
                //generateMesh();
            }
        }

        void connectPosY(TerrainChunk* chunk) {

            std::scoped_lock lock(mtx);
            
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