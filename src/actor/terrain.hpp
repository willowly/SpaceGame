
#pragma once
#include "graphics/model.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"
#include "SimplexNoise.h"

using std::unique_ptr;

class Terrain : Actor {

    std::vector<float> terrainData;


    public:
        unique_ptr<Model> dynamicModel;
        Material* material;
        int size = 8;
        float scale = 0.2;


    Terrain() {
        dynamicModel = std::make_unique<Model>();
        dynamicModel->setDynamicDraw();
        model = dynamicModel.get();
    }

    void generate() {
        terrainData.clear();

        const SimplexNoise simplex;

        for (int z = 0; z < size; z++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float noise = simplex.noise(x*scale,y*scale,z*scale);
                    terrainData.push_back(noise);
                    std::cout << StringHelper::toString(vec3(x,y,z)) << noise << std::endl;
                }
            }
        }
    }

    virtual void render(Camera& camera,float dt) {
        //std::cout << "rendering terrain:" << std::endl;
        model->render(vec3(0,0,0),camera,*material);

        int i = 0;
        for (int z = 0; z < size; z++)
        {
            for (int y = 0; y < size; y++)
            {
                for (int x = 0; x < size; x++)
                {
                    float noise = terrainData[i];
                    Debug::drawPoint(vec3(x,y,z),Color(noise,noise,noise));
                    i++;
                }
            }
        }
        
    }


    void setConfig(int config) {
        model->vertices.clear();
        model->faces.clear();
        model->normals.clear();
        model->normals.push_back(vec3(0,1,0));
        model->uvs.clear();
        model->uvs.push_back(glm::vec2(0,0));
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
        Model::Face face;
        while(i < 100) //break out if theres a problem lol
        {
            int edge = tris[i];
            if(edge == -1) break;
            std::cout << edge << ", ";
            model->vertices.push_back(getEdgePos(edge));
            int vertIndex = i % 3;
            face.vertexIndices[vertIndex] = i;
            face.normalIndicies[vertIndex] = 0;
            face.uvIndicies[vertIndex] = 0;
            Debug::drawPoint(getEdgePos(edge));
            if(vertIndex == 2) { 
                model->faces.push_back(face);
            }
            i++;
        }
        model->updateData();
        model->bindData();
        
        
    }

    vec3 getEdgePos(int index) {
        switch(index) {
            case 0:
                return vec3(0.5f,0,0);
            case 1:
                return vec3(1,0.5f,0);
            case 2:
                return vec3(0.5f,1,0);
            case 3:
                return vec3(0,0.5f,0);
            case 4:
                return vec3(0.5f,0,1);
            case 5:
                return vec3(1,0.5f,1);
            case 6:
                return vec3(0.5f,1,1);
            case 7:
                return vec3(0,0.5f,1);
            case 8:
                return vec3(0,0,0.5f);
            case 9:
                return vec3(1,0,0.5f);
            case 10:
                return vec3(1,1,0.5f);
            case 11:
                return vec3(0,1,0.5f);

        }
        return vec3(0,0,0);
    }

};