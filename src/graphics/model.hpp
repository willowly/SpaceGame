#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "helper/file-helper.hpp"
#include "helper/string-helper.hpp"
#include "vulkan.hpp"
#include <map>

using std::vector,glm::vec3,glm::vec2;

class Model {
    public:

        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 texCoord;

            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(Vertex, pos);

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(Vertex, normal);

                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

                return attributeDescriptions;
            }
        };

        struct MeshDataConstant {
            glm::mat4 matrix;
        };

        enum RenderMode {
            Solid,
            Wireframe
        };
        vector<Vertex> vertices;
        vector<uint16_t> indices;
        RenderMode renderMode = RenderMode::Solid;
        
        bool vkBuffersLoaded;
        VkDevice device;
        VkBuffer meshBuffer;
        VkDeviceMemory meshBufferMemory;
        VkDeviceSize indexOffset;

        Model() {
            
        }

        ~Model() {
            if(vkBuffersLoaded) {
                vkDestroyBuffer(device, meshBuffer, nullptr);
                vkFreeMemory(device, meshBufferMemory, nullptr);
            }
        }

        

        void loadFromFile(string path) {
            vertices.clear();

            std::vector<vec3> positions;
            std::vector<vec3> normals;
            std::vector<vec2> uvs;
            for(string line : FileHelper::readToStrings(path)) {
                if(line.length() > 2) {
                    vector<string> lineSegments = StringHelper::split(line," ");

                    // vertex positions
                    if(lineSegments[0] == "v") {
                        if(lineSegments.size() == 4) {
                            positions.push_back(parseVector3(lineSegments));
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

                        std::map<Vertex,int> vertexMap;

                        if(lineSegments.size() == 4) {
                            for (int i = 1; i < 4; i++)
                            {
                                Vertex vertex;
                                string lineSegment = lineSegments[i];
                                vector<string> vertexSegments = StringHelper::split(lineSegment,"/");
                                
                                vertex.pos = positions[stoi(vertexSegments[0])-1]; //not 0 indexed
                                vertex.texCoord = uvs[stoi(vertexSegments[1])-1];
                                vertex.normal = normals[stoi(vertexSegments[2])-1];

                                if(vertexMap.contains(vertex)) {
                                    indices.push_back(vertexMap[vertex]);
                                } else {
                                    vertices.push_back(vertex);
                                    int index = vertices.size()-1;
                                    vertexMap[vertex] = index;
                                    indices.push_back(index);
                                }
                            }

                        } else{
                            modelLoadError(path,"not exactly 3 vertices on this face components (mesh must be triangulated) (found " + std::to_string(lineSegments.size() - 1) + ")",line);
                        }
                        
                    }
                } else {
                    modelLoadError(path,"weird line length",line);
                }
            }
        }

        // void loadQuad() {

        //     vertices.push_back(vec3(1.0,1.0,0));
        //     vertices.push_back(vec3(-1.0,1.0,0));
        //     vertices.push_back(vec3(1.0,-1.0,0));
        //     vertices.push_back(vec3(-1.0,-1.0,0));

        //     normals.push_back(vec3(0.0,0.0,1.0));

        //     uvs.push_back(vec2(1,1));
        //     uvs.push_back(vec2(0,1));
        //     uvs.push_back(vec2(1,0));
        //     uvs.push_back(vec2(0,0));

        //                     // vertex       //normal     /uv
        //     faces.push_back(Face(0,1,2,     0,0,0,       0,1,2));
        //     faces.push_back(Face(1,3,2,     0,0,0,       1,3,2));
            
        // }

        // void loadLine(vec3 a,vec3 b) {
        //     vertices.push_back(a);
        //     vertices.push_back(b);

        //     normals.push_back(vec3(0.0,0.0,1.0));
        //     uvs.push_back(vec2(0,0));

        //     //                  vertex      normal       uv
        //     faces.push_back(Face(0,1,0,     0,0,0,       0,0,0));
        // }

        void createBuffers(Vulkan& vulkan) {

            device = vulkan.getDevice();
            vulkan.createMeshBuffers<Vertex>(meshBuffer,meshBufferMemory,vertices,indices);
            indexOffset = sizeof(vertices[0]) * vertices.size();
            vkBuffersLoaded = true;

        }

        void recordDraw(Vulkan& vulkan) {
            VkCommandBuffer commandBuffer = vulkan.getCurrentCommandBuffer();
            vkCmdBindIndexBuffer(commandBuffer, meshBuffer, indexOffset, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
        }

        void setStaticDraw() {
            
        };

        void setDynamicDraw() {
           
        };

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