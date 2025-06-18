#pragma once
#include <vector>
#include <include/glm/glm.hpp>
#include <include/glm/gtc/matrix_transform.hpp>
#include <include/glm/gtc/type_ptr.hpp>
#include "helper/file-helper.hpp"
#include "helper/string-helper.hpp"

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
            PositionUV,
        };
        vector<float> vertexData;
       //vector<int> indexData;




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
                            vertices.push_back(parseVector(lineSegments));
                        } else{
                            modelLoadError(path,"not exactly 3 vertex position components (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                    }

                    // vertex normals
                    if(lineSegments[0] == "vn") {
                        if(lineSegments.size() == 4) {
                            normals.push_back(parseVector(lineSegments));
                        } else{
                            modelLoadError(path,"not exactly 3 vertex normal components (found " + std::to_string(lineSegments.size() - 1) + ")",line);
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
                                
                                face.vertexIndices[i] = stoi(vertexSegments[0]);
                                face.uvIndicies = stoi(vertexSegments[2]);
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
                    }
                    for (int i = 0; i < 3; i++)
                    {
                        vertexData.push_back(vertices[face.uvIndicies[i]].x);
                        vertexData.push_back(vertices[face.uvIndicies[i]].y);
                        vertexData.push_back(vertices[face.uvIndicies[i]].z);
                    }
                    
                    
                    break;
            }
        }

    private:
        //expects the first segment to be the line identifier
        static vec3 parseVector(vector<string> lineSegments) {
            return vec3(
                std::stof(lineSegments[1]),
                std::stof(lineSegments[2]),
                std::stof(lineSegments[3])
                );
        }
        static void modelLoadError(string path,string error,string line) {
            std::cout << "[ERROR] ("<<path<<") Model load error: " + error << "\n\t" << line << std::endl;
        }
};