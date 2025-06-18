#pragma once
#include <vector>
#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include "helper/file-helper.hpp"
#include "helper/string-helper.hpp"
#include "graphics/shader.hpp"

using std::vector,glm::vec3,glm::vec2;

class Model {
    public:
        vector<vec3> vertices;
        vector<vec3> normals;
        vector<vec2> uvs;

        struct Face {
            int vertexIndices[3];
            int normalIndicies[3];
            int uvIndicies[3];
        };

        
        vector<Face> faces;
        
        enum DataFormat {
            Position,
            PositionUV,
        };
        vector<float> vertexData;
        unsigned int VAO;
        unsigned int VBO;

        Shader* shader;
       //vector<int> indexData;

        Model() {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);  
        }



        void loadFromFile(string path,DataFormat dataFormat) {
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
            updateData(dataFormat);
            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexData.size(), vertexData.data(), GL_STATIC_DRAW);
            

            // set up indicies
            // unsigned int EBO;
            // glGenBuffers(1, &EBO);
            // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indicesVector.size(), indicesVector.data(), GL_STATIC_DRAW);

            // // set up the vertex attributes
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
            // glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3* sizeof(float)));
            glEnableVertexAttribArray(1);
            
        }

        void render(glm::mat4 view,glm::mat4 model,glm::mat4 projection) {
            if(shader == nullptr) {
                std::cout << "[ERROR] shader is null" << std::endl;
            }
            shader->use();
            shader->setMat4("view",view);
            shader->setMat4("model",model);
            shader->setMat4("projection",projection);

            glDrawArrays(GL_TRIANGLES, 0, vertexData.size());
        }

        void updateData(DataFormat dataFormat) {
            vertexData.clear();
            for(auto face : faces) {
                addFace(face,dataFormat);
            }
        }

        void addFace(Face face,DataFormat dataFormat) {
            switch (dataFormat) {
                case DataFormat::PositionUV:
                    for (int i = 0; i < 3; i++)
                    {
                        vertexData.push_back(vertices[face.vertexIndices[i]].x);
                        vertexData.push_back(vertices[face.vertexIndices[i]].y);
                        vertexData.push_back(vertices[face.vertexIndices[i]].z);
                        vertexData.push_back(uvs[face.uvIndicies[i]].x);
                        vertexData.push_back(uvs[face.uvIndicies[i]].y);
                    }
                    break;
                case DataFormat::Position:
                    for (int i = 0; i < 3; i++)
                    {
                        vertexData.push_back(vertices[face.vertexIndices[i]].x);
                        vertexData.push_back(vertices[face.vertexIndices[i]].y);
                        vertexData.push_back(vertices[face.vertexIndices[i]].z);
                    }
                    break;
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
            std::cout << "[ERROR] ("<<path<<") Model load error: " + error << "\n\t" << line << std::endl;
        }
};