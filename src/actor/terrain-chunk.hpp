#include "helper/terrain-helper.hpp"
#include "graphics/mesh.hpp"
#include <vector>
#include "glm/glm.hpp"
#include "item/item.hpp"
#include "SimplexNoise.h"
#include "terrain-structs.hpp"

#include <map>


#define SURFACE_LVL 0.5

class TerrainChunk {

    struct VoxelData {
        float amount = 0;
        int type;
        int verticesStart = 0;
        int verticesEnd = 0;
    };

    vec3 offset;

    std::vector<VoxelData> terrainData;
    std::vector<TerrainVertex> vertices;
    std::vector<uint16_t> indices;
    int meshState = -1;
    MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
    bool meshOutOfDate = false;
    TerrainType terrainTypes[8];

    int chunkSize = 30;
    float cellSize = 0.5f;

    int getPointIndex(int x,int y,int z) {
        x = std::clamp(x,0,chunkSize-1);
        y = std::clamp(y,0,chunkSize-1);
        z = std::clamp(z,0,chunkSize-1);
        return x + y * chunkSize + z * chunkSize * chunkSize;
    }

    float getPoint(int x,int y,int z) {
        x = std::clamp(x,0,chunkSize-1);
        y = std::clamp(y,0,chunkSize-1);
        z = std::clamp(z,0,chunkSize-1);
        // if(x < 0 || x >= chunkSize) return 0;
        // if(y < 0 || y >= chunkSize) return 0;
        // if(z < 0 || z >= chunkSize) return 0;
        return terrainData[x + y * chunkSize + z * chunkSize * chunkSize].amount;
    }

    bool getPointInside(int x,int y,int z) {
        
        return getPoint(x,y,z) > SURFACE_LVL;
    }

    vec3 getCellChunkPos(ivec3 cell) {
        vec3 cellPos = (vec3)cell + vec3(0.5);
        cellPos = cellPos*cellSize;
        return cellPos;
    }

    public:
        TerrainChunk(vec3 offset,int chunkSize,float cellSize) : offset(offset), chunkSize(chunkSize), cellSize(cellSize) {}

        void generateData(GenerationSettings settings) {

            terrainTypes[0] = settings.stoneType;
            terrainData.resize(chunkSize*chunkSize*chunkSize);

            meshBuffer[0] = settings.meshBuffer;

            const SimplexNoise simplex;

            float radius = chunkSize*0.5f;

            int i = 0;
            for (int z = 0; z < chunkSize; z++)
            {
                int percent = ((float)z/chunkSize)*100;
                //std::cout << "generating terrain " << percent << "%" << std::endl;
                for (int y = 0; y < chunkSize; y++)
                {

                    for (int x = 0; x < chunkSize; x++)
                    {
                        vec3 samplePos = vec3(x,y,z);
                        samplePos += offset;
                        samplePos /= settings.noiseScale;
                        float noise = simplex.fractal(5,samplePos.x,0,samplePos.z) * chunkSize;
                        noise *= 0.5f;
                        noise += 5;
                        

                        terrainData[i].amount = std::clamp(noise - y,0.0f,1.0f);
                        terrainData[i].type = 0;
                        //float distance = glm::length(vec3(x-radius,y-radius,z-radius));
                        //float radiusInfluence = (radius-distance)/radius;
                        //terrainData[i].amount = noise;// * (radiusInfluence+0.5f);
                        i++;
                    }
                }
            }
            // generateOre(1,30,0.3,offset,chunk);
            // generateOre(2,60,0.4,offset,chunk);
        }

        //position is in chunk space
        TerraformResults terraformSphere(vec3 pos,float radius,float change) {


            TerraformResults results;
            return results;

            // float clock = (float)glfwGetTime();
            // auto posCellSpace = getCellAtWorldPos(pos);
            // auto radiusCellSpace = radius/cellSize;
            // std::cout << radiusCellSpace << std::endl;
            // std::cout << posCellSpace.z-radiusCellSpace << std::endl;
            // std::cout << posCellSpace.z+radiusCellSpace << std::endl;
            // for (int z = floor(std::max(0.0f,posCellSpace.z-radiusCellSpace)); z <= ceil(std::min((float)chunkSize,posCellSpace.z+radiusCellSpace)); z++)
            // {
            //     for (int y = floor(std::max(0.0f,posCellSpace.y-radiusCellSpace)); y <= ceil(std::min((float)chunkSize,posCellSpace.y+radiusCellSpace)); y++)
            //     {
            //         for (int x = floor(std::max(0.0f,posCellSpace.x-radiusCellSpace)); x <= ceil(std::min((float)chunkSize,posCellSpace.x+radiusCellSpace)); x++)
            //         {
            //             int i = getPointIndex(x,y,z);
                        
            //             vec3 cellPos = getCellWorldPos(vec3(x,y,z));
            //             float dist = glm::distance(cellPos,pos);
            //             float influence = (radius-dist)/radius;
                        
            //             if(influence > 0) {
            //                 float old = terrainData[i].amount;
            //                 terrainData[i].amount += change * influence;
            //                 terrainData[i].amount = std::min(std::max(terrainData[i].amount,0.0f),1.0f);
            //                 int terrainTypeID = terrainData[i].type;
                            
            //                 if(old > SURFACE_LVL && terrainData[i].amount < SURFACE_LVL) {
            //                     results.item = terrainTypes[terrainTypeID].item;
            //                 }
            //             }
            //             i++;
            //         }
            //     }
            // }
            // return results;
            // std::cout << "terraform time: " << (float)glfwGetTime() - clock << std::endl;
        }

        

        void addRenderables(Vulkan* vulkan,float dt,vec3 position,Material material) {
            if(vertices.size() == 0 || indices.size() == 0) return;

            
            vulkan->addMesh(meshBuffer[0],material,glm::translate(glm::mat4(1.0f),position+(offset*cellSize)));
            // if(meshState == -1) {
            //     meshBuffer[0] = vulkan->createMeshBuffers(vertices,indices);
            //     //meshBuffer[1] = vulkan->createMeshBuffers(vertices,indices);
            //     meshOutOfDate = false;
            //     meshState = 0;
            // } else {
            //     if(meshOutOfDate) {
            //         meshState++;
            //         if(meshState >= FRAMES_IN_FLIGHT) meshState = 0;
            //         vulkan->updateMeshBuffer(meshBuffer[meshState],vertices,indices);
            //         meshOutOfDate = false;
            //     }
            // }
            // vulkan->addMesh(meshBuffer[meshState],material,glm::translate(glm::mat4(1.0f),position+(offset*cellSize)));
        }
        void generateMesh() {
            float clock = (float)glfwGetTime();
            vertices.clear();
            indices.clear();

            int i = 0;
            for (int z = 0; z < chunkSize; z++)
            {
                int percent = ((float)z/chunkSize)*100;
                //std::cout << "generating mesh " << percent << "%" << std::endl;
                for (int y = 0; y < chunkSize; y++)
                {
                    
                    for (int x = 0; x < chunkSize; x++)
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
                        addCell(config,vec3(x,y,z));
                        i++;
                    }
                }
            }

            //consolodate normals
            //std::vector<TerrainVertex> newVertices;
            unordered_map<vec3,std::vector<TerrainVertex*>> vertexMap;
            for(auto& i : indices) {
                auto& vertex = vertices[i];
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
            
            // vertices = std::move(newVertices);
            meshOutOfDate = true;
            std::cout << "generate time: " << (float)glfwGetTime() - clock << std::endl;
        }

        void addCell(int config,vec3 cellPos) {
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
            int startIndex = indices.size();
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

                
                int closestCellIndex;
                if(a > b) {
                    closestCellIndex = getPointIndex((int)aPos.x,(int)aPos.y,(int)aPos.z);
                } else {
                    closestCellIndex = getPointIndex((int)bPos.x,(int)bPos.y,(int)bPos.z);
                }
                int vertIndex = i % 3;

                TextureID texture = terrainTypes[0].texture;
                
                auto vertex = TerrainVertex((getEdgePos(edge,t) + cellPos)*cellSize);
                textureIDVec[vertIndex] = texture;
                vertex.oreBlend[vertIndex] = 1;
                
                vertices.push_back(vertex);
                face[vertIndex] = vertices.size()-1;
                indices.push_back(vertices.size()-1);
                if(vertIndex == 2) {
                    vec3 normal = MathHelper::normalFromPlanePoints(vertices[face[0]].pos,vertices[face[1]].pos,vertices[face[2]].pos);
                    vertices[face[0]].normal = normal;
                    vertices[face[0]].textureID = textureIDVec;
                    vertices[face[1]].normal = normal;
                    vertices[face[1]].textureID = textureIDVec;
                    vertices[face[2]].normal = normal;
                    vertices[face[2]].textureID = textureIDVec;
                }
                i++;
            }
            terrainData[cellIndex].verticesEnd = indices.size();
            
            
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