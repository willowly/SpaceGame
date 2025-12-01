#pragma once

#include "vulkan.hpp"


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

struct SkyboxMaterial {
    TextureID texture;
    SkyboxMaterial(TextureID texture) : texture(texture) {}
};

class Skybox {

    MeshBuffer mesh;
    public:
        TextureID top;
        Material material;
        // TextureID bottom;
        // TextureID left;
        // TextureID right;
        // TextureID front;
        // TextureID back;
    


    bool renderToRender;

    void loadResources(Vulkan& vulkan) {
        auto pipeline = vulkan.createManagedPipeline<SkyboxVertex>(Vulkan::vertCodePath("skybox"),Vulkan::fragCodePath("skybox"));
        material = vulkan.createMaterial(pipeline,SkyboxMaterial(top));

        std::vector<SkyboxVertex> vertices = {
            SkyboxVertex(vec3(1,1,1),  vec2(0,0),0),
            SkyboxVertex(vec3(-1,1,1), vec2(1,0),0),
            SkyboxVertex(vec3(1,-1,1), vec2(0,1),0),
            SkyboxVertex(vec3(-1,-1,1),vec2(1,1),0)
        };

        std::vector<uint16_t> indices = {
            0,1,2,
            2,3,4
        };

        mesh = vulkan.createMeshBuffers(vertices,indices);

    }

    void addRenderables(Vulkan& vulkan,Camera& camera) {
        auto mat = glm::mat4(1.0f);
        mat = glm::translate(mat,camera.position);
        vulkan.addMesh(mesh,material,mat);
    }
};