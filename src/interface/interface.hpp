#pragma once

#include "graphics/mesh-data.hpp"
#include <glm/glm.hpp>
#include <helper/math-helper.hpp>

#include "engine/debug.hpp"


using glm::ivec2, glm::vec2;

struct UIVertex {
    vec2 pos;
    vec2 uv;
    UIVertex () {}
    UIVertex (vec2 pos,vec2 uv) : pos(pos), uv(uv) {}

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(UIVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(UIVertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(UIVertex, uv);

        return attributeDescriptions;
    }
};

struct UIMaterialData {
    
};

struct UIExtraPushData {
    vec4 color;
    TextureID texture;
    UIExtraPushData(vec4 color,TextureID texture) : color(color), texture(texture) {}
};

class Interface {
    public:

        MeshBuffer quadMesh;
        Material material = Material::none;
        ivec2 screenSize;
        bool readyToRender = false;

        Interface() {

        }

        void loadRenderResources(Vulkan& vulkan) {
            MeshData<UIVertex> data;
            data.vertices = std::vector<UIVertex> {
                UIVertex(vec2(0,0),vec2(0,1)),
                UIVertex(vec2(0,1),vec2(0,0)),
                UIVertex(vec2(1,0),vec2(1,1)),
                UIVertex(vec2(1,1),vec2(1,0))
            };
            data.indices = std::vector<uint16_t> {
                0,1,2,1,2,3
            };
            quadMesh = vulkan.createMeshBuffers<UIVertex>(data.vertices,data.indices);
            PipelineOptions options;
            options.depthTestEnabled = VK_FALSE;
            options.blend = VK_TRUE;
            VkPipeline pipeline = vulkan.createManagedPipeline<UIVertex>(Vulkan::vertCodePath("ui"),Vulkan::fragCodePath("ui"),options);
            UIMaterialData materialData;
            material = vulkan.createMaterial(pipeline,materialData);
            readyToRender = true;
        }

        void drawRectCentered(Vulkan& vulkan,vec2 position,vec2 size,vec2 anchor,Color color,TextureID texture) {
            drawRect(vulkan,position,size,anchor,vec2(0.5,0.5),color,texture);
        }
        
        void drawRect(Vulkan& vulkan,vec2 position,vec2 size,vec2 anchor,Color color,TextureID texture) {
            drawRect(vulkan,position,size,anchor,vec2(0,0),color,texture);
        }

        void drawRect(Vulkan& vulkan,vec2 position,vec2 size,vec2 anchor,vec2 pivot,Color color,TextureID texture) {
            if(!readyToRender) {
                Debug::warn("Interface not ready to render");
                return;
            }

            position += (vec2)vulkan.getScreenSize() * anchor; 
            position -= pivot * size;

            glm::mat4 model = glm::translate(glm::mat4(1.0f),vec3(position,1));
            model = glm::scale(model,vec3(size,1));
            vulkan.addMeshWithData<UIExtraPushData>(quadMesh,material,UIExtraPushData(color.asVec4(),texture),model);
        }





};