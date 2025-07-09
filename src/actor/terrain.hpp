
#pragma once
#include "graphics/model.hpp"
#include "actor/actor.hpp"
#include "helper/terrain-helper.hpp"
#include "engine/debug.hpp"

using std::unique_ptr;

class Terrain : Actor {

    public:
        unique_ptr<Model> dynamicModel;


    Terrain() {
        dynamicModel = std::make_unique<Model>();
        dynamicModel->setDynamicDraw();
        model = dynamicModel.get();
    }

    virtual void render(Camera& camera,float dt) {
        model->render(vec3(0,0,0),camera,*Debug::getShader());
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
        std::cout << "tris: ";
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
                std::cout << "(face)" << std::endl;
            }
            i++;
        }
        std::cout << std::endl;

        model->updateData();
        
        
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