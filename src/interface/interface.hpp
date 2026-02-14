#pragma once

#include "graphics/mesh-data.hpp"
#include <glm/glm.hpp>
#include <helper/math-helper.hpp>

#include "engine/debug.hpp"

#include "helper/rect.hpp"
#include "helper/sprite.hpp"


using glm::ivec2, glm::vec2;

struct UIVertex {
    vec2 pos= {};
    vec2 uv = {};
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
    vec2 spriteOffset= {};
    vec2 spriteSize = {};
    TextureID texture; // at the end for alignment
    UIExtraPushData(vec4 color,Sprite sprite) : color(color), spriteOffset(sprite.rect.position),spriteSize(sprite.rect.size),texture(sprite.texture) {}
};

class Interface {
    public:

        MeshBuffer quadMesh;
        Material material = Material::none;
        bool readyToRender = false;
        float scale = 6;

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
            options.depthTestEnabled = VK_TRUE;
            options.depthCompareOp = VK_COMPARE_OP_ALWAYS;
            options.blend = VK_TRUE;
            VkPipeline pipeline = vulkan.createManagedPipeline<UIVertex>(Vulkan::vertCodePath("ui"),Vulkan::fragCodePath("ui"),options);
            UIMaterialData materialData;
            material = vulkan.createMaterial(pipeline,materialData);
            readyToRender = true;
        }


        void drawRect(Vulkan& vulkan,Rect rect,Color color,Sprite sprite) {
            if(!readyToRender) {
                Debug::warn("Interface not ready to render");
                return;
            } 
            rect.position *= scale;

            glm::mat4 model = glm::translate(glm::mat4(1.0f),vec3(rect.position,1));
            model = glm::scale(model,vec3(rect.size * scale,1));
            vulkan.addMesh<UIExtraPushData>(quadMesh,material,UIExtraPushData(color.asVec4(),sprite),model);
        }

};

struct DrawContext {
    private:
        Interface& interface;
        Vulkan& vulkan;
        Input& input;
    public:
        DrawContext(Interface& interface,Vulkan& vulkan,Input& input) : interface(interface), vulkan(vulkan), input(input) {}

        void drawRect(Rect rect,Color color,Sprite sprite) {
            interface.drawRect(vulkan,rect,color,sprite);
        }

        vec2 getScreenSize() {
            return vulkan.getScreenSize() / interface.scale;
        }

        vec2 getMousePosition() {
            return (input.currentMousePosition / interface.scale);
        }

        Input& getInput() {
            return input;
        }

        bool mouseInside(Rect rect) {
            vec2 mouse = getMousePosition();
            if(mouse.x < rect.position.x) return false;
            if(mouse.y < rect.position.y) return false;
            if(mouse.x > rect.position.x + rect.size.x) return false;
            if(mouse.y > rect.position.y + rect.size.y) return false;
            return true;
        }

        bool mouseLeftClicked() {
            return input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1);
        }
        bool mouseRightClicked() {
            return input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_2);
        }
};