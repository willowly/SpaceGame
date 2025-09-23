#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "helper/file-helper.hpp"
#include "helper/string-helper.hpp"
#include "graphics/material.hpp"
#include "camera.hpp"

using std::vector,glm::vec3,glm::vec2;

class Model {
    public:

        enum RenderMode {
            Solid,
            Wireframe
        };
        vector<vec3> vertices;
        vector<vec3> normals;
        vector<vec2> uvs;
        RenderMode renderMode = RenderMode::Solid;

        struct Face {
            int vertexIndices[3];
            int normalIndicies[3];
            int uvIndicies[3];

            Face(int v0,int v1,int v2,int n0,int n1,int n2,int uv0,int uv1,int uv2) {
                vertexIndices[0] = v0;
                vertexIndices[1] = v1;
                vertexIndices[2] = v2;
                normalIndicies[0] = n0;
                normalIndicies[1] = n1;
                normalIndicies[2] = n2;
                uvIndicies[0] = uv0;
                uvIndicies[1] = uv1;
                uvIndicies[2] = uv2;
            }
            Face() {}
        };

        GLenum drawType = GL_STATIC_DRAW;

        
        vector<Face> faces;

        vector<float> vertexData;
        unsigned int VAO;
        unsigned int VBO;
       //vector<int> indexData;

        Model() {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
        }

        ~Model() {
            glDeleteVertexArrays(1,&VAO);
            glDeleteBuffers(1,&VBO);
        }

        

        void loadFromFile(string path) {
            vertices.clear();
            normals.clear();
            faces.clear();
            for(string line : FileHelper::readToStrings(path)) {
                if(line.length() > 2) {
                    vector<string> lineSegments = StringHelper::split(line," ");

                    // vertex positions
                    if(lineSegments[0] == "v") {
                        if(lineSegments.size() == 4) {
                            vertices.push_back(parseVector3(lineSegments));
                        } else{
                            modelLoadError(path,"not exactly 3 vertex position components (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                    }
                    

                    // vertex normals
                    if(lineSegments[0] == "vn") {
                        if(lineSegments.size() == 4) {
                            normals.push_back(parseVector3(lineSegments));
                        } else{
                            modelLoadError(path,"not exactly 3 vertex normal components (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                    }

                    if(lineSegments[0] == "vt") {
                        if(lineSegments.size() == 3) {
                            uvs.push_back(parseVector2(lineSegments));
                        } else{
                            modelLoadError(path,"not exactly 2 vertex uv components (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                    }

                    //faces
                    if(lineSegments[0] == "f") {
                        if(lineSegments.size() == 4) {
                            Face face;
                            for (int i = 1; i < 4; i++)
                            {
                                string lineSegment = lineSegments[i];
                                vector<string> vertexSegments = StringHelper::split(lineSegment,"/");
                                
                                face.vertexIndices[i-1] = stoi(vertexSegments[0])-1; //not 0 indexed
                                face.uvIndicies[i-1] = stoi(vertexSegments[1])-1;
                                face.normalIndicies[i-1] = stoi(vertexSegments[2])-1;
                            }

                            faces.push_back(face);

                        } else{
                            modelLoadError(path,"not exactly 3 vertices on this face components (mesh must be triangulated) (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                        
                    }
                } else {
                    modelLoadError(path,"weird line length",line);
                }
            }
            updateData();
            bindData();
        }

        void loadQuad() {

            vertices.push_back(vec3(1.0,1.0,0));
            vertices.push_back(vec3(-1.0,1.0,0));
            vertices.push_back(vec3(1.0,-1.0,0));
            vertices.push_back(vec3(-1.0,-1.0,0));

            normals.push_back(vec3(0.0,0.0,1.0));

            uvs.push_back(vec2(1,1));
            uvs.push_back(vec2(0,1));
            uvs.push_back(vec2(1,0));
            uvs.push_back(vec2(0,0));

                            // vertex       //normal     /uv
            faces.push_back(Face(0,1,2,     0,0,0,       0,1,2));
            faces.push_back(Face(1,3,2,     0,0,0,       1,3,2));

            updateData();
            bindData();
            
        }

        void loadLine(vec3 a,vec3 b) {
            vertices.push_back(a);
            vertices.push_back(b);

            normals.push_back(vec3(0.0,0.0,1.0));
            uvs.push_back(vec2(0,0));

            //                  vertex      normal       uv
            faces.push_back(Face(0,1,0,     0,0,0,       0,0,0));
            updateData();
            bindData();
        }

        void setStaticDraw() {
            drawType = GL_STATIC_DRAW;
        };

        void setDynamicDraw() {
            drawType = GL_DYNAMIC_DRAW;
        };

        void bindData() {
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), drawType);
            

            // set up indicies
            // unsigned int EBO;
            // glGenBuffers(1, &EBO);
            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indicesVector.size(), indicesVector.data(), GL_STATIC_DRAW);

            // // set up the vertex attributes
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6* sizeof(float)));
            glEnableVertexAttribArray(2);
        }

        void bind() {
            glBindVertexArray(VAO);
        }


        void render(glm::mat4 view,glm::mat4 model,glm::mat4 projection,Material& material) {

            bind();
            material.use();
            material.shader->setViewModelProjection(view,model,projection);

            renderNoBind();
        }

        void render(glm::mat4 transformation,Camera& camera,Material& material) {
            render(camera.getViewMatrix(),transformation,camera.getProjectionMatrix(),material);
        }

        void render(glm::vec3 position,Camera& camera,Material& material) {
            render(camera.getViewMatrix(),glm::translate(glm::mat4(1.0f),position),camera.getProjectionMatrix(),material);
        }

        

        void render(glm::mat4 view,glm::mat4 model,glm::mat4 projection,Shader& shader) {

            bind();
            shader.use();
            shader.setViewModelProjection(view,model,projection);

            renderNoBind();
        }

        void render(glm::mat4 transformation,Camera& camera,Shader& shader) {
            render(camera.getViewMatrix(),transformation,camera.getProjectionMatrix(),shader);
        }

        void render(glm::vec3 position,Camera& camera,Shader& shader) {
            render(camera.getViewMatrix(),glm::translate(glm::mat4(1.0f),position),camera.getProjectionMatrix(),shader);
        }

        void render() {

            bind();
            renderNoBind();
        }

        void renderNoBind() {
            switch(renderMode) {
                case RenderMode::Solid:
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    break;
                case RenderMode::Wireframe:
                    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
                    break;
            }
            glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/8);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }



        void updateData() {
            vertexData.clear();
            for(auto face : faces) {
                addFaceToData(face);
            }
        }

        void addFaceToData(Face face) {
            for (int i = 0; i < 3; i++)
            {
                vertexData.push_back(vertices[face.vertexIndices[i]].x);
                vertexData.push_back(vertices[face.vertexIndices[i]].y);
                vertexData.push_back(vertices[face.vertexIndices[i]].z);
                vertexData.push_back(normals[face.normalIndicies[i]].x);
                vertexData.push_back(normals[face.normalIndicies[i]].y);
                vertexData.push_back(normals[face.normalIndicies[i]].z);
                vertexData.push_back(uvs[face.uvIndicies[i]].x);
                vertexData.push_back(uvs[face.uvIndicies[i]].y);
            }
        }

    private:
        //expects the first segment to be the line identifier
        static vec3 parseVector3(vector<string> lineSegments) {
            return vec3(
                std::stof(lineSegments[1]),
                std::stof(lineSegments[2]),
                std::stof(lineSegments[3])
                );
        }
        static vec2 parseVector2(vector<string> lineSegments) {
            return vec2(
                std::stof(lineSegments[1]),
                std::stof(lineSegments[2])
                );
        }
        static void modelLoadError(string path,string error,string line) {
            std::cout << "[WARNING] ("<<path<<") Model load error: " + error << "\n\t" << line << std::endl;
        }
};