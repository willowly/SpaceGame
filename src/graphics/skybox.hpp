#pragma once

#include "vulkan.hpp"
#include "engine/debug.hpp"
#include <array>

struct SkyboxVertex {
    vec3 pos;
    vec2 texCoord;
    int face;
    SkyboxVertex(vec3 pos,vec2 texCoord,int face) : pos(pos), texCoord(texCoord), face(face) {}
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(SkyboxVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(SkyboxVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(SkyboxVertex, texCoord);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[2].offset = offsetof(SkyboxVertex, face);

        return attributeDescriptions;
    }
};

struct SkyboxMaterialData {
    TextureID top;
    TextureID bottom;
    TextureID left;
    TextureID right;
    TextureID front;
    TextureID back;
    SkyboxMaterialData() {}
    SkyboxMaterialData(TextureID top,TextureID bottom,TextureID left,TextureID right,TextureID front,TextureID back) : top(top),bottom(bottom),left(left),right(right),front(front),back(back) {}
};

class Skybox {

    MeshBuffer mesh;
    bool readyToRender = false;
    Material material = Material::none;

    public:
    



    void loadResources(Vulkan& vulkan,SkyboxMaterialData materialData) {

        PipelineOptions opts;
        opts.depthCompareOp = VK_COMPARE_OP_EQUAL;
        auto pipeline = vulkan.createManagedPipeline<SkyboxVertex>(Vulkan::vertCodePath("skybox"),Vulkan::fragCodePath("skybox"),opts);
        material = vulkan.createMaterial(pipeline,materialData);

        

        std::vector<SkyboxVertex> vertices;

        std::vector<uint16_t> indices;

        addFace(vertices,indices,glm::quat(vec3(0.0f,glm::radians(-90.0f),0.0f)),0);
        addFace(vertices,indices,glm::quat(vec3(0.0f,0.0f,glm::radians(180.0f))),1);

        

        addFace(vertices,indices,glm::quat(vec3(0.0f,glm::radians(180.0f),glm::radians(-90.0f))),2);
        addFace(vertices,indices,glm::quat(vec3(0.0f,0.0f,glm::radians(90.0f))),3);

        addFace(vertices,indices,glm::quat(vec3(glm::radians(-90.0f),0.0f,glm::radians(180.0f))),4);
        addFace(vertices,indices,glm::quat(vec3(glm::radians(90.0f),0.0f,0.0f)),5);

        mesh = vulkan.createMeshBuffers(vertices,indices);
        readyToRender = true;

    }

    void addFace(std::vector<SkyboxVertex>& vertices,std::vector<uint16_t>& indices,quat rot,int face) {
        uint16_t i = vertices.size();
        vertices.push_back(SkyboxVertex(rot * vec3(1,1,1),  vec2(0,0),face));
        vertices.push_back(SkyboxVertex(rot * vec3(-1,1,1), vec2(1,0),face));
        vertices.push_back(SkyboxVertex(rot * vec3(1,1,-1), vec2(0,1),face));
        vertices.push_back(SkyboxVertex(rot * vec3(-1,1,-1),vec2(1,1),face));
        
        std::vector<int> newIndices = {0,1,2,1,2,3};
        for(auto index : newIndices) {
            indices.push_back(i+(uint16_t)index);
        }
        // 0,1,2,
        //     1,2,3,
    }

    void addRenderables(Vulkan& vulkan,Camera& camera) {

        if(!readyToRender) {
            Debug::warn("skybox not ready to render");
            return;
        }

        auto mat = glm::mat4(1.0f);
        mat = glm::translate(mat,camera.position);
        mat = glm::scale(mat,vec3(10.0f));
        vulkan.addMesh(mesh,material,mat);
    }
};