
#pragma once
#include "graphics/model.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"

using std::unique_ptr;

class Terrain : RigidbodyActor {

    std::vector<float> terrainData;


    public:
        unique_ptr<Model> dynamicModel;
        Material* material;
        int size = 50;
        float noiseScale = 100;
        float surfaceLevel = 0.5;
        float cellSize = 0.5f;


    Terrain() : RigidbodyActor(nullptr,nullptr) {
        dynamicModel = std::make_unique<Model>();
        dynamicModel->setDynamicDraw();
        model = dynamicModel.get();
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
                    std::cout << radiusInfluence  << " " << distance << " " << x << "," << y << "," << z << "" << std::endl;
                    terrainData.push_back(noise * (radiusInfluence+0.5f));
                }
            }
        }
    }

    virtual void render(Camera& camera,float dt) {
        //std::cout << "rendering terrain:" << std::endl;
        model->render(position,camera,*material);
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
    }


    void raycast(Ray ray,float distance) {
        
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

    void terraformSphere(vec3 pos,int radius,float change) {
        int i = 0;
        for (int z = 0; z < size; z++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    vec3 cellPos = vec3(x,y,z)*cellSize;
                    float dist = glm::distance(cellPos,pos);
                    float influence = (radius-dist)/radius;
                    if(influence > 0) {
                        //std::cout << "influence: " << influence << " data: " << terrainData[i] << std::endl;
                        terrainData[i] += change * influence;
                        terrainData[i] = std::min(std::max(terrainData[i],0.0f),1.0f);
                        //std::cout << "data after: " << terrainData[i] << std::endl;
                    }
                    i++;
                }
            }
        }
    }

};