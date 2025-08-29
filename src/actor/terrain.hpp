
#pragma once
#include "graphics/model.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"

using std::unique_ptr;

class Terrain : public Actor {

    std::vector<float> terrainData;


    public:
        unique_ptr<Model> dynamicModel;
        int size = 10;
        float noiseScale = 30;
        float surfaceLevel = 0.5;
        float cellSize = 0.5f;

    

    Terrain() : Actor(nullptr,nullptr) {
        
        dynamicModel = std::make_unique<Model>();
        dynamicModel->setDynamicDraw();
        model = dynamicModel.get();

        generateData();
        generateMesh();
    }

    Terrain(vec3 position,quat rotation) : Terrain() {
        this->position = position;
        this->rotation = rotation;
    }

    void generateData() {
        terrainData.clear();

        const SimplexNoise simplex;

        float radius = size*0.5f;
        for (int z = 0; z < size; z++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float noise = simplex.fractal(5,x/noiseScale,y/noiseScale,z/noiseScale);
                    float distance = glm::length(vec3(x-radius,y-radius,z-radius));
                    float radiusInfluence = (radius-distance)/radius;
                    terrainData.push_back(noise * (radiusInfluence+0.5f));
                }
            }
        }
    }

    virtual void render(Camera& camera,float dt) {
        float clock = (float)glfwGetTime();
        if(material == nullptr) {
            std::cout << "null material" << std::endl;
            return;
        }
        model->render(position,camera,*material);
        //std::cout << "render time: " << (float)glfwGetTime() - clock << std::endl;
    }

    int getPointIndex(int x,int y,int z) {
        return x + y * size + z * size * size;
    }

    float getPoint(int x,int y,int z) {
        if(x < 0 || x >= size) return 0;
        if(y < 0 || y >= size) return 0;
        if(z < 0 || z >= size) return 0;
        return terrainData[x + y * size + z * size * size];
    }

    bool getPointInside(int x,int y,int z) {
        
        return getPoint(x,y,z) > surfaceLevel;
    }


    void generateMesh() {
        float clock = (float)glfwGetTime();
        model->vertices.clear();
        model->faces.clear();
        model->normals.clear();
        model->uvs.clear();
        model->uvs.push_back(glm::vec2(0,0));

        int i = 0;
        for (int z = -1; z < size; z++)
        {
            for (int y = -1; y < size; y++)
            {
                for (int x = -1; x < size; x++)
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


        model->updateData();
        model->bindData();
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
        int startIndex = model->vertices.size();
        Model::Face face;
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
            model->vertices.push_back((getEdgePos(edge,t) + cellPos)*cellSize);
            int vertIndex = i % 3;
            face.vertexIndices[vertIndex] =  i + startIndex;
            face.normalIndicies[vertIndex] = 0;
            face.uvIndicies[vertIndex] = 0;
            if(vertIndex == 2) {
                vec3 normal = MathHelper::normalFromPlanePoints(model->vertices[face.vertexIndices[0]],model->vertices[face.vertexIndices[1]],model->vertices[face.vertexIndices[2]]);
                model->normals.push_back(normal);
                face.normalIndicies[0] = model->normals.size() - 1;
                face.normalIndicies[1] = model->normals.size() - 1;
                face.normalIndicies[2] = model->normals.size() - 1;
                model->faces.push_back(face);
            }
            i++;
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

    ivec3 getCellAtWorldPos(vec3 pos) {
        return glm::floor(inverseTransformPoint(pos)/cellSize);
    }

    vec3 getCellWorldPos(ivec3 cell) {
        vec3 cellPos = (vec3)cell + vec3(0.5);
        cellPos = transformPoint(cellPos*cellSize);
        return cellPos;
    }

    virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {
        int i = 0;
        float clock = (float)glfwGetTime();
        std::optional<RaycastHit> result = std::nullopt;
        Ray localRay = Ray(inverseTransformPoint(ray.origin),inverseTransformDirection(ray.direction));
        for (auto& face : model->faces)
        {
            vec3 a = model->vertices[face.vertexIndices[0]];
            vec3 b = model->vertices[face.vertexIndices[1]];
            vec3 c = model->vertices[face.vertexIndices[2]];

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
            i++;
                
        }
        //std::cout << "raycast time: " << (float)glfwGetTime() - clock << std::endl;
        return result;
    }
    
    //need to manually regenerate the mesh (in case things want to)
    void terraformSphere(vec3 pos,float radius,float change) {

        float clock = (float)glfwGetTime();
        auto posCellSpace = getCellAtWorldPos(pos);
        auto radiusCellSpace = radius/cellSize;
        std::cout << radiusCellSpace << std::endl;
        std::cout << posCellSpace.z-radiusCellSpace << std::endl;
        std::cout << posCellSpace.z+radiusCellSpace << std::endl;
        for (int z = floor(std::max(0.0f,posCellSpace.z-radiusCellSpace)); z < ceil(std::min((float)size,posCellSpace.z+radiusCellSpace)); z++)
        {
            for (int y = floor(std::max(0.0f,posCellSpace.y-radiusCellSpace)); y < ceil(std::min((float)size,posCellSpace.y+radiusCellSpace)); y++)
            {
                for (int x = floor(std::max(0.0f,posCellSpace.x-radiusCellSpace)); x < ceil(std::min((float)size,posCellSpace.x+radiusCellSpace)); x++)
                {
                    int i = getPointIndex(x,y,z);
                    
                    vec3 cellPos = getCellWorldPos(vec3(x,y,z));
                    float dist = glm::distance(cellPos,pos);
                    float influence = (radius-dist)/radius;
                    
                    if(influence > 0) {
                        terrainData[i] += change * influence;
                        terrainData[i] = std::min(std::max(terrainData[i],0.0f),1.0f);
                    }
                    i++;
                }
            }
        }
        std::cout << "terraform time: " << (float)glfwGetTime() - clock << std::endl;
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