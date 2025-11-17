
#pragma once
#include "graphics/mesh.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"

using std::unique_ptr;

class Terrain : public Actor {


    struct VoxelData {
        float amount = 0;
        int verticesStart = 0;
        int verticesEnd = 0;
    };

    std::vector<VoxelData> terrainData;
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    Terrain() : Actor() {

    }

    public:
        int meshState = -1;
        MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
        bool meshOutOfDate;
        Material material = Material::none;
        int size = 30;
        float noiseScale = 100;
        float surfaceLevel = 0.5;
        float cellSize = 0.5f;

    

    void generateData() {
        terrainData.resize(size*size*size);

        const SimplexNoise simplex;

        float radius = size*0.5f;

        int i = 0;
        for (int z = 0; z < size; z++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float noise = simplex.fractal(5,x/noiseScale,y/noiseScale,z/noiseScale);
                    float distance = glm::length(vec3(x-radius,y-radius,z-radius));
                    float radiusInfluence = (radius-distance)/radius;
                    terrainData[i].amount = noise * (radiusInfluence+0.5f);
                    i++;
                }
            }
        }
    }

    virtual void addRenderables(Vulkan* vulkan,float dt) {
        if(vertices.size() == 0 || indices.size() == 0) return;
        if(meshState == -1) {
            meshBuffer[0] = vulkan->createMeshBuffers(vertices,indices);
            meshBuffer[1] = vulkan->createMeshBuffers(vertices,indices);
            meshOutOfDate = false;
            meshState = 0;
        } else {
            if(meshOutOfDate) {
                meshState++;
                if(meshState >= FRAMES_IN_FLIGHT) meshState = 0;
                vulkan->updateMeshBuffer(meshBuffer[meshState],vertices,indices);
                meshOutOfDate = false;
            }
        }
        vulkan->addMesh(meshBuffer[meshState],material,glm::translate(glm::mat4(1.0f),position));
        //std::cout << "render time: " << (float)glfwGetTime() - clock << std::endl;
    }

    int getPointIndex(int x,int y,int z) {
        return x + y * size + z * size * size;
    }

    float getPoint(int x,int y,int z) {
        if(x < 0 || x >= size) return 0;
        if(y < 0 || y >= size) return 0;
        if(z < 0 || z >= size) return 0;
        return terrainData[x + y * size + z * size * size].amount;
    }

    bool getPointInside(int x,int y,int z) {
        
        return getPoint(x,y,z) > surfaceLevel;
    }


    void generateMesh() {
        float clock = (float)glfwGetTime();
        vertices.clear();
        indices.clear();

        int i = 0;
        for (int z = 0; z < size; z++)
        {
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
                    addCell(config,vec3(x,y,z));
                    i++;
                }
            }
        }
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
        int startIndex = vertices.size();
        int cellIndex = getPointIndex(cellPos.x,cellPos.y,cellPos.z);
        terrainData[cellIndex].verticesStart = startIndex;
        int face[3];
        while(i < 100) //break out if theres a problem lol
        {
            int edge = tris[i];
            if(edge == -1) break;
            vec3 aPos = getEdgePos(edge,0)+cellPos;
            vec3 bPos = getEdgePos(edge,1)+cellPos;
            float a = getPoint((int)aPos.x,(int)aPos.y,(int)aPos.z);
            float b = getPoint((int)bPos.x,(int)bPos.y,(int)bPos.z);
            
            
            
            float t = (surfaceLevel - a)/(b-a);
            t = std::min(std::max(t,0.0f),1.0f);
            vertices.push_back(Vertex((getEdgePos(edge,t) + cellPos)*cellSize));
            int vertIndex = i % 3;
            face[vertIndex] = vertices.size()-1;
            indices.push_back(vertices.size()-1);
            if(vertIndex == 2) {
                vec3 normal = MathHelper::normalFromPlanePoints(vertices[face[0]].pos,vertices[face[1]].pos,vertices[face[2]].pos);
                vertices[face[0]].normal = normal;
                vertices[face[1]].normal = normal;
                vertices[face[2]].normal = normal;
            }
            i++;
        }
        terrainData[cellIndex].verticesEnd = vertices.size();
        
        
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

    ivec3 getCellAtWorldPos(vec3 pos) {
        return glm::floor(inverseTransformPoint(pos)/cellSize);
    }

    vec3 getCellWorldPos(ivec3 cell) {
        vec3 cellPos = (vec3)cell + vec3(0.5);
        cellPos = transformPoint(cellPos*cellSize);
        return cellPos;
    }

    virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {

        std::optional<RaycastHit> result = std::nullopt;
        Ray localRay = Ray(inverseTransformPoint(ray.origin),inverseTransformDirection(ray.direction));
        for(size_t i = 0;i+2 < indices.size();i += 3)
        {
            vec3 a = vertices[indices[i]].pos;
            vec3 b = vertices[indices[i+1]].pos;
            vec3 c = vertices[indices[i+2]].pos;

            auto hitopt = Physics::intersectRayTriangle(a,b,c,localRay);
            if(hitopt) {
                
                auto hit = hitopt.value();
                if(hit.distance <= dist) {
                    hit.point = transformPoint(hit.point);
                    hit.normal = transformDirection(hit.normal);
                    result = hit;
                    dist = hit.distance; //distance stays the same when transformed
                }
            }
                
        }
        //std::cout << "raycast time: " << (float)glfwGetTime() - clock << std::endl;
        return result;
    }

    virtual void collideBasic(Actor* actor,float radius) {
        vec3 localActorPosition = inverseTransformPoint(actor->position);
        auto posCellSpace = getCellAtWorldPos(actor->position);
        auto radiusCellSpace = radius/cellSize;
        for (int z = std::max(0,(int)floor(posCellSpace.z-radiusCellSpace)); z <= std::min(size-1,(int)ceil(posCellSpace.z+radiusCellSpace)); z++)
        {
            for (int y = std::max(0,(int)floor(posCellSpace.y-radiusCellSpace)); y <= std::min(size-1,(int)ceil(posCellSpace.y+radiusCellSpace)); y++)
            {
                for (int x = std::max(0,(int)floor(posCellSpace.x-radiusCellSpace)); x <= std::min(size-1,(int)ceil(posCellSpace.x+radiusCellSpace)); x++)
                {
                    auto voxel = terrainData[getPointIndex(x,y,z)];
                    for (int i = voxel.verticesStart;i < voxel.verticesEnd;i += 3)
                    {
                        vec3 a = vertices[i].pos;
                        vec3 b = vertices[i+1].pos;
                        vec3 c = vertices[i+2].pos;

                        auto contact_opt = Physics::intersectSphereTri(localActorPosition,radius,a,b,c);
                        if(contact_opt) {
                            
                            auto contact = contact_opt.value();
                            Physics::resolveBasic(localActorPosition,contact);
                            
                        }
                            
                    } 
                }
            }
        }
        actor->position = transformPoint(localActorPosition);
    }
    
    //need to manually regenerate the mesh (in case things want to)
    void terraformSphere(vec3 pos,float radius,float change) {

        float clock = (float)glfwGetTime();
        auto posCellSpace = getCellAtWorldPos(pos);
        auto radiusCellSpace = radius/cellSize;
        std::cout << radiusCellSpace << std::endl;
        std::cout << posCellSpace.z-radiusCellSpace << std::endl;
        std::cout << posCellSpace.z+radiusCellSpace << std::endl;
        for (int z = floor(std::max(0.0f,posCellSpace.z-radiusCellSpace)); z <= ceil(std::min((float)size,posCellSpace.z+radiusCellSpace)); z++)
        {
            for (int y = floor(std::max(0.0f,posCellSpace.y-radiusCellSpace)); y <= ceil(std::min((float)size,posCellSpace.y+radiusCellSpace)); y++)
            {
                for (int x = floor(std::max(0.0f,posCellSpace.x-radiusCellSpace)); x <= ceil(std::min((float)size,posCellSpace.x+radiusCellSpace)); x++)
                {
                    int i = getPointIndex(x,y,z);
                    
                    vec3 cellPos = getCellWorldPos(vec3(x,y,z));
                    float dist = glm::distance(cellPos,pos);
                    float influence = (radius-dist)/radius;
                    
                    if(influence > 0) {
                        terrainData[i].amount += change * influence;
                        terrainData[i].amount = std::min(std::max(terrainData[i].amount,0.0f),1.0f);
                    }
                    i++;
                }
            }
        }
        std::cout << "terraform time: " << (float)glfwGetTime() - clock << std::endl;
    }

    static std::unique_ptr<Terrain> makeInstance(Material material,vec3 position = vec3(0)) {
        auto ptr = new Terrain();
        ptr->material = material;
        ptr->generateData();
        ptr->generateMesh();
        return std::unique_ptr<Terrain>(ptr);
    }


    void drawCellsOnRay(Ray ray,float dist) {

        
        //assert(abs(glm::length(ray.direction) - 1) > 0.001);
        
        Debug::drawRay(ray.origin,ray.direction*10.0f,Color::white);
        Ray cellSpaceRay = Ray(transformPoint(ray.origin),transformDirection(ray.direction));
        cellSpaceRay.origin = cellSpaceRay.origin/cellSize;
        float cellDist = dist/cellSize;
        float yxSlope = cellSpaceRay.direction.y/abs(cellSpaceRay.direction.x);
        float zxSlope = cellSpaceRay.direction.z/abs(cellSpaceRay.direction.x);
        std::cout << cellDist*cellSpaceRay.direction.x << std::endl;
        int xDir = ray.direction.x > 0 ? 1 : -1;
        float xOffset = MathHelper::fromFloor(cellSpaceRay.origin.x*xDir);
        for(int i = 1;((i-1)*cellSize) <= (abs(cellSpaceRay.direction.x)*dist+xOffset);i++) {

            
            
            ivec3 cellPos = ivec3(
                    xDir*floor(i+cellSpaceRay.origin.x)-1,
                    MathHelper::integerBelow(((i-xOffset)*yxSlope)+cellSpaceRay.origin.y),
                    MathHelper::integerBelow(((i-xOffset)*zxSlope)+cellSpaceRay.origin.z)
            );
            Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::green);
        }

        float xySlope = cellSpaceRay.direction.x/cellSpaceRay.direction.y;
        float zySlope = cellSpaceRay.direction.z/cellSpaceRay.direction.y;
        for(int i = 1;i <= ceil(cellDist*cellSpaceRay.direction.y)+1;i++) {

            float offset = MathHelper::fromFloor(cellSpaceRay.origin.y);

            ivec3 cellPos = ivec3(
                    MathHelper::integerBelow(((i-offset)*xySlope)+cellSpaceRay.origin.x),
                    floor(i+cellSpaceRay.origin.y)-1,
                    MathHelper::integerBelow(((i-offset)*zySlope)+cellSpaceRay.origin.z)
            );
            Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::magenta);
        }

        float xzSlope = cellSpaceRay.direction.x/cellSpaceRay.direction.z;
        float yzSlope = cellSpaceRay.direction.y/cellSpaceRay.direction.z;
        for(int i = 1;i <= ceil(cellDist*cellSpaceRay.direction.z)+1;i++) {

            float offset = MathHelper::fromFloor(cellSpaceRay.origin.z);

            ivec3 cellPos = ivec3(
                    MathHelper::integerBelow(((i-offset)*xzSlope)+cellSpaceRay.origin.x),
                    MathHelper::integerBelow(((i-offset)*yzSlope)+cellSpaceRay.origin.y),
                    floor(i+cellSpaceRay.origin.z)-1
            );
            Debug::drawCube(getCellWorldPos(cellPos),vec3(cellSize),Color::red);
        }
    }

};