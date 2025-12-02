#pragma once


#define IMGUI_IMPL_VULKAN_USE_VOLK
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"


#include <GLFW/glfw3.h>

#include "helper/string-helper.hpp"



#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <format>
#include <set>
#include "helper/file-helper.hpp"
#include <glm/glm.hpp>
#include "stb_image.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <chrono>

#include "camera.hpp"

#define FRAMES_IN_FLIGHT 2
#define DESCRIPTOR_COUNT 128

using std::string,glm::vec3,glm::vec2,glm::ivec2,glm::mat4,glm::quat;

struct SceneDataBufferObject {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 screen;
    vec3 viewDir;
};

 
class Vulkan;

struct Buffer {

    friend class Vulkan;
        int allocationIndex = -1;
        VkBuffer buffer;
        VkDeviceMemory memory;
};

struct Image {
    friend class Vulkan;
    private:
        
        VkImage image;
        VkDeviceMemory memory;

};

typedef uint TextureID;
typedef VkDeviceAddress MaterialHandle;


struct LitMaterialData {

    LitMaterialData() {
        
    }

    LitMaterialData(TextureID texture,vec4 color = vec4(1)) : texture(texture), color(color) {

    }
        
    TextureID texture;
    vec4 color = vec4(1);

    
};

class Material {
    friend Vulkan;
    VkPipeline pipeline = VK_NULL_HANDLE;
    MaterialHandle data = 0;
    Material(VkPipeline pipeline, MaterialHandle data) : pipeline(pipeline), data(data) {}
    Material() {}
    public:
        static const Material none;
};

const Material Material::none = Material();

struct MeshBuffer : Buffer {
    VkDeviceSize indexOffset = 0;
    uint32_t indexCount = 0;

    MeshBuffer() {

    }

    MeshBuffer(const Buffer& o) {
        allocationIndex = o.allocationIndex;
        buffer = o.buffer;
        memory = o.memory;
    }
};

struct MeshPushConstant {
    glm::mat4 matrix = glm::mat4(1.0f);
    uint frameIndex;
    MaterialHandle materialData;
    char extraData[48];

    MeshPushConstant(glm::mat4 matrix) : matrix(matrix) {

    }
};

struct PipelineOptions {
    VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    VkBool32 depthTestEnabled = VK_TRUE;
    VkCompareOp depthCompareOp = VK_COMPARE_OP_LESS;
    VkBlendOp blendOp = VK_BLEND_OP_ADD;
    VkBool32 blend = VK_FALSE;
};

class Vulkan {
    public:
        bool enableValidationLayers = true;

        GLFWwindow* window;

        Vulkan(string name,GLFWwindow* window) {
            this->window = window;
            //global
            createVkInstance(name);
            createSurface();
            pickPhysicalDevice();
            createLogicalDevice();
            createSwapChain();
            createImageViews();
            createRenderPass();
            createCommandPool();
            createCommandBuffer();
            createDepthResources();
            createFrameBuffers();
            createSyncObjects();
            
            // per-shader?
            createDescriptorSetLayout();
            createGraphicsPipelineLayout();

            // per model?

            // depends, modelviewprojection buffers are global but textures are per-material
            createUniformBuffers();
            createDescriptorPool();
            createDescriptorSets(); // also includes the texture

            //initImGui();
            
        }

        ~Vulkan() {

            cleanupSwapChain();

            vkDestroyDescriptorPool(device, descriptorPool, nullptr);

            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

            for(auto texture : textures) {
                destroyTextureResources(texture);
            }

            for(auto pipeline : managedPipelines) {
                vkDestroyPipeline(device,pipeline,nullptr);
            }
            
            

            vkDestroySwapchainKHR(device, swapChain, nullptr);
            for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(device, acquireSemaphores[i], nullptr);
                vkDestroyFence(device, inFlightFences[i], nullptr);
                destroyBuffer(uniformBuffers[i]);
            }
            
            for (size_t i = 0; i < submitSemaphores.size(); i++)
            {
                vkDestroySemaphore(device, submitSemaphores[i], nullptr);
            }

            for (auto buffer_opt : allocatedBuffers)
            {
                if(buffer_opt) {
                    auto buffer = buffer_opt.value();
                    vkDestroyBuffer(device,buffer.buffer,nullptr);
                    vkFreeMemory(device,buffer.memory,nullptr);
                }
            }
            
            
            vkDestroyPipelineLayout(device,pipelineLayout,nullptr);

            vkDestroyCommandPool(device, commandPool, nullptr);
            vkDestroyRenderPass(device, renderPass, nullptr);

            vkDestroyDevice(device, nullptr);
            vkDestroySurfaceKHR(vkInstance, vkSurface, nullptr);
            vkDestroyInstance(vkInstance, nullptr);


        }

        VkDevice getDevice() {
            return device;
        }

        void addMesh(MeshBuffer& mesh,Material material, glm::mat4 matrix = glm::mat4(1.0f)) {

            renderObjects.push_back(RenderObject(mesh,matrix,material.pipeline,material.data));

        }

        template<typename T>
        void addMeshWithData(MeshBuffer& mesh,Material material,T data, glm::mat4 matrix = glm::mat4(1.0f)) {

            static_assert(sizeof(T) <= 48);
            auto renderObject = RenderObject(mesh,matrix,material.pipeline,material.data);
            std::memcpy(&renderObject.extraData,&data,sizeof(T));
            renderObjects.push_back(renderObject);

        }

        vec2 getScreenSize() {
            return screenSize;
        }

        void render(const Camera& camera) {

           
            //wait for previous frame to be finished drawing
            vkWaitForFences(device, 1, &inFlightFences[frameIndex], VK_TRUE, UINT64_MAX);
            
            // scene data buffer
            updateUniformBuffer(frameIndex,camera);
            
            VkResult result;
            if(!frameBufferResized) {
                result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, acquireSemaphores[frameIndex], VK_NULL_HANDLE, &imageIndex);
            }
            if (frameBufferResized || result == VK_ERROR_OUT_OF_DATE_KHR) {
                frameBufferResized = false;
                recreateSwapChain();
                return;
            } else if (result != VK_SUCCESS && result != VK_NOT_READY && result != VK_SUBOPTIMAL_KHR && result != VK_TIMEOUT) {
                throw std::runtime_error("failed to acquire swap chain image!");
            }
            
            vkResetFences(device, 1, &inFlightFences[frameIndex]);

            VkCommandBuffer commandBuffer = commandBuffers[frameIndex];
            vkResetCommandBuffer(commandBuffer, 0);
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.0f, 0.0f, 0.01f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);


            VkPipeline currentPipeline = VK_NULL_HANDLE;
            VkBuffer currentMeshBuffer = VK_NULL_HANDLE;

            for (auto& renderObject : renderObjects)
            {
                MeshBuffer meshBuffer = renderObject.meshBuffer;
                if(currentMeshBuffer != meshBuffer.buffer) {
                    
                    VkBuffer vertexBuffers[] = {meshBuffer.buffer};
                    VkDeviceSize offsets[] = {0};
                    vkCmdBindVertexBuffers(commandBuffer,0,1,vertexBuffers,offsets);
                    vkCmdBindIndexBuffer(commandBuffer, meshBuffer.buffer, meshBuffer.indexOffset, VK_INDEX_TYPE_UINT16);
                }

                if(renderObject.pipeline != currentPipeline) {
                    vkCmdBindPipeline(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,renderObject.pipeline);
                    currentPipeline = renderObject.pipeline;
                }
                
                MeshPushConstant pushConstant(renderObject.matrix);
                pushConstant.frameIndex = frameIndex;
                pushConstant.materialData = renderObject.materialData;
                std::copy(std::begin(renderObject.extraData),std::end(renderObject.extraData),std::begin(pushConstant.extraData));
                //std::cout << sizeof(MeshPushConstant) << std::endl;
                vkCmdPushConstants(commandBuffer,pipelineLayout,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,0,sizeof(MeshPushConstant),&pushConstant);
                
                
                vkCmdDrawIndexed(commandBuffer, meshBuffer.indexCount, 1, 0, 0, 0);

            }

            vkCmdEndRenderPass(commandBuffer);

            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  
            VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &acquireSemaphores[frameIndex];
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffers[frameIndex];
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &submitSemaphores[imageIndex];

            if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[frameIndex]) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit draw command buffer!");
            }

            VkPresentInfoKHR presentInfo{};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.waitSemaphoreCount = 1;
            presentInfo.pWaitSemaphores = &submitSemaphores[imageIndex];
            presentInfo.swapchainCount = 1;
            presentInfo.pSwapchains = &swapChain;
            presentInfo.pImageIndices = &imageIndex;
            presentInfo.pResults = nullptr; // Optional

            vkQueuePresentKHR(presentQueue, &presentInfo);

            frameIndex = (frameIndex + 1) % FRAMES_IN_FLIGHT;
            testIndex = (testIndex + 1) % 5;
        }

        void clearObjects() {
            renderObjects.clear();
        }

        TextureID loadTextureFile(string path) {
            int texWidth, texHeight, texChannels;
            stbi_set_flip_vertically_on_load(true);
            stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels) {
                throw std::runtime_error("failed to load texture image!");
            }
            TextureID id = loadTextureMemory(texWidth,texHeight,texChannels,pixels);
            stbi_image_free(pixels);
            return id;
        }

        TextureID loadTextureMemory(int texWidth, int texHeight, int texChannels,void* data) {
            TextureResources texture;
            texture.image = createTextureImage(texWidth,texHeight,texChannels,data);
            texture.view = createTextureImageView(texture.image);
            texture.sampler = createTextureSampler(texture.image);
            textures.push_back(texture);
            updateDescriptorSet();
            
            return textures.size() - 1;
        }

        void waitIdle() {
            vkDeviceWaitIdle(device);
        }

        void setFrameBufferResized() {
            frameBufferResized = true;
        }

        template<typename MaterialData>
        Material createMaterial(VkPipeline pipeline,MaterialData material) {

            VkDeviceSize bufferSize = sizeof(MaterialData);

            Buffer stagingBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            
            //copy the memory into it
            void* data;
            vkMapMemory(device, stagingBuffer.memory, 0, bufferSize, 0, &data);
                memcpy(data, &material, (size_t) bufferSize);
            vkUnmapMemory(device, stagingBuffer.memory);

            // create the vertex buffer
            Buffer buffer = createManagedBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR);
            VkBufferDeviceAddressInfoKHR addressInfo{};
            addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            addressInfo.buffer = buffer.buffer;

            // copy the data over
            copyBuffer(stagingBuffer, buffer, bufferSize);
            
            // destroy the staging buffer
            destroyBuffer(stagingBuffer);
            
            auto bda = vkGetBufferDeviceAddressKHR(device, &addressInfo);

            return Material(pipeline,bda);
        }

        template<typename Vertex>
        MeshBuffer createMeshBuffers(std::vector<Vertex>& vertices,std::vector<uint16_t>& indices) {
            
            VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
            VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
            VkDeviceSize bufferSize = vertexBufferSize + indexBufferSize;

            // create the staging buffer
            Buffer stagingBuffer = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

            
            //copy the memory into it
            void* data;
            vkMapMemory(device, stagingBuffer.memory, 0, vertexBufferSize, 0, &data);
                memcpy(data, vertices.data(), (size_t) vertexBufferSize);
            vkUnmapMemory(device, stagingBuffer.memory);
            vkMapMemory(device, stagingBuffer.memory, vertexBufferSize, indexBufferSize, 0, &data);
                memcpy(data, indices.data(), (size_t) indexBufferSize);
            vkUnmapMemory(device, stagingBuffer.memory);

            // create the vertex buffer
            Buffer buffer = createManagedBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            // copy the data over
            copyBuffer(stagingBuffer, buffer, bufferSize);

            MeshBuffer meshBuffer(buffer);
            meshBuffer.indexOffset = sizeof(vertices[0]) * vertices.size();
            meshBuffer.indexCount = static_cast<uint32_t>(indices.size());

            // destroy the staging buffer
            destroyBuffer(stagingBuffer);

            return meshBuffer;
        }

        template<typename Vertex>
        void updateMeshBuffer(MeshBuffer& meshBuffer,std::vector<Vertex>& vertices,std::vector<uint16_t>& indices) {

            destroyBuffer(meshBuffer);
            meshBuffer = createMeshBuffers(vertices,indices);

        }

        template<typename Vertex>
        VkPipeline createManagedPipeline(string vertCodePath,string fragCodePath,PipelineOptions pipelineOptions = PipelineOptions()) {
            auto vertShaderCode = FileHelper::readBinary(vertCodePath);
            auto fragShaderCode = FileHelper::readBinary(fragCodePath);
            return createManagedPipelineFromMemory<Vertex>(vertShaderCode,fragShaderCode,pipelineOptions);
        }

        template<typename Vertex>
        VkPipeline createManagedPipelineFromMemory(std::vector<char> vertShaderCode,std::vector<char> fragShaderCode,PipelineOptions pipelineOptions = PipelineOptions()) {

            VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
            VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

            VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
            vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vertShaderStageInfo.module = vertShaderModule;
            vertShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
            fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            fragShaderStageInfo.module = fragShaderModule;
            fragShaderStageInfo.pName = "main";

            VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

            VkPipelineDepthStencilStateCreateInfo depthStencil{};
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = pipelineOptions.depthTestEnabled;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = pipelineOptions.depthCompareOp;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.minDepthBounds = 0.0f; // Optional
            depthStencil.maxDepthBounds = 1.0f; // Optional
            depthStencil.stencilTestEnable = VK_FALSE;
            depthStencil.front = {}; // Optional
            depthStencil.back = {}; // Optional

            std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
            };

            VkPipelineDynamicStateCreateInfo dynamicState{};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
            dynamicState.pDynamicStates = dynamicStates.data();

            auto bindingDescription = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount = 1;
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()); //Yummy;
            vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data(); // Optional

            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            inputAssembly.topology = pipelineOptions.topology;
            inputAssembly.primitiveRestartEnable = VK_FALSE;

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = (float) swapChainExtent.width;
            viewport.height = (float) swapChainExtent.height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = swapChainExtent;

            VkPipelineViewportStateCreateInfo viewportState{};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor;


            VkPipelineRasterizationStateCreateInfo rasterizer{};
            rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            rasterizer.depthClampEnable = VK_FALSE;
            rasterizer.polygonMode = pipelineOptions.polygonMode;
            rasterizer.lineWidth = 1.0f;
            rasterizer.cullMode = VK_CULL_MODE_NONE;
            rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizer.depthBiasEnable = VK_FALSE;
            rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
            rasterizer.depthBiasClamp = 0.0f;           // Optional
            rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional

            VkPipelineMultisampleStateCreateInfo multisampling{};
            multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            multisampling.sampleShadingEnable = VK_FALSE;
            multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            multisampling.minSampleShading = 1.0f; // Optional
            multisampling.pSampleMask = nullptr; // Optional
            multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
            multisampling.alphaToOneEnable = VK_FALSE; // Optional

            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = pipelineOptions.blend;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // Optional
            colorBlendAttachment.colorBlendOp = pipelineOptions.blendOp; // Optional
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
            colorBlendAttachment.alphaBlendOp = pipelineOptions.blendOp; // Optional

            VkPipelineColorBlendStateCreateInfo colorBlending{};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment;
            colorBlending.blendConstants[0] = 0.0f; // Optional
            colorBlending.blendConstants[1] = 0.0f; // Optional
            colorBlending.blendConstants[2] = 0.0f; // Optional
            colorBlending.blendConstants[3] = 0.0f; // Optional


            rasterizer.rasterizerDiscardEnable = VK_FALSE;


            VkGraphicsPipelineCreateInfo pipelineInfo{};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.stageCount = 2;
            pipelineInfo.pStages = shaderStages;

            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pDepthStencilState = &depthStencil; // Optional
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;

            pipelineInfo.layout = pipelineLayout;

            pipelineInfo.renderPass = renderPass;
            pipelineInfo.subpass = 0;

            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
            pipelineInfo.basePipelineIndex = -1; // Optional

            VkPipeline pipeline;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
            
            managedPipelines.push_back(pipeline);

            return pipeline;
        }

        static string vertCodePath(string name) {
            return "shaders/compiled/" + name + "_vert.spv";
        }

        static string fragCodePath(string name) {
            return "shaders/compiled/" + name + "_frag.spv";
        }

        
    private:
        struct QueueFamilies {
            std::optional<uint32_t> graphicsFamily;
            std::optional<uint32_t> presentFamily;
            bool isComplete() {
                return graphicsFamily.has_value() && presentFamily.has_value();
            }
        };

        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> presentModes;
        };

        struct RenderObject {
            MeshBuffer& meshBuffer;
            glm::mat4 matrix;
            VkPipeline pipeline;
            MaterialHandle materialData;
            char extraData[48];

            RenderObject(MeshBuffer& meshBuffer,glm::mat4 matrix,VkPipeline pipeline,MaterialHandle materialData) : meshBuffer(meshBuffer), matrix(matrix), pipeline(pipeline), materialData(materialData) {

            }
        };

        const std::vector<const char*> validationLayers = {
            //"VK_LAYER_KHRONOS_validation"
        };

        std::vector<const char*> deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
            VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
            
        };

        struct TextureResources {

            Image image;
            VkSampler sampler;
            VkImageView view;
                
        };

        uint32_t imageIndex;
        
        VkInstance vkInstance;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkSurfaceKHR vkSurface;
        VkSwapchainKHR swapChain;
        std::vector<VkImage> swapChainImages;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImageView> swapChainImageViews;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool;

        VkSemaphore acquireSemaphores[FRAMES_IN_FLIGHT];
        std::vector<VkSemaphore> submitSemaphores;
        VkFence inFlightFences[FRAMES_IN_FLIGHT];
        VkCommandBuffer commandBuffers[FRAMES_IN_FLIGHT];
        uint frameIndex = 0;
        uint testIndex = 0;

        bool frameBufferResized = false;

        bool hasSwapChain = false;

        VkPipelineLayout pipelineLayout;
        
        VkDescriptorSetLayout descriptorSetLayout;
        VkDescriptorPool descriptorPool;
        VkDescriptorSet descriptorSet;

        std::vector<TextureResources> textures;

        Buffer uniformBuffers[FRAMES_IN_FLIGHT+1];
        void* uniformBuffersMapped[FRAMES_IN_FLIGHT+1];

        Image depthImage;
        VkImageView depthImageView;

        vec2 screenSize;

        std::vector<std::optional<Buffer>> allocatedBuffers;
        int totalAllocatedBuffersCount = 0; //for the unique ids

        std::vector<VkPipeline> managedPipelines;

        std::vector<RenderObject> renderObjects;

        void createVkInstance(string name) {

            if(volkInitialize() != VK_SUCCESS) {
                throw std::runtime_error("failed to initalize volk");
            }

            if (enableValidationLayers && !checkValidationLayerSupport()) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            //set up app info
            VkApplicationInfo appInfo{};
            appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName = name.c_str();
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName = "No Engine";
            appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion = VK_API_VERSION_1_0;

            //set up create info
            VkInstanceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            createInfo.pApplicationInfo = &appInfo;

            // connect to glfw
            uint32_t glfwExtensionCount = 0;
            const char** glfwExtensions;

            glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

            std::vector<const char*> requiredExtensions;

            for(uint32_t i = 0; i < glfwExtensionCount; i++) {
                requiredExtensions.emplace_back(glfwExtensions[i]);
            }

            requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
            requiredExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            requiredExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            requiredExtensions.emplace_back(VK_KHR_DEVICE_GROUP_CREATION_EXTENSION_NAME);

            createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

            createInfo.enabledExtensionCount = (uint32_t) requiredExtensions.size();
            createInfo.ppEnabledExtensionNames = requiredExtensions.data();

            //validation layers
            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            VkResult result = vkCreateInstance(&createInfo, nullptr, &vkInstance);
            if(result != VK_SUCCESS) {
                std::cout << result << std::endl;
                throw std::runtime_error("failed to create vulkan instance");
            }

            volkLoadInstance(vkInstance);
            //vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)(vkGetInstanceProcAddr(vkInstance,"vkSetDebugUtilsObjectNameEXT"));
        }

        void pickPhysicalDevice() {
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }
            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(vkInstance, &deviceCount, devices.data());
            
            for (const auto& device : devices) {
                if (isDeviceSuitable(device)) {
                    physicalDevice = device;
                    break;
                }
            }

            if (physicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error("failed to find a suitable GPU!");
            }

        }

        QueueFamilies findQueueFamilies(VkPhysicalDevice device ) {

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            
            
            QueueFamilies result;
            int i = 0;
            for (const auto& queueFamily : queueFamilies) {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    result.graphicsFamily = i;
                }
                

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, vkSurface, &presentSupport);

                if (presentSupport) {
                    result.presentFamily = i;
                }

                if (result.isComplete()) {
                    break; //early exit
                }

                i++;
            }
            return result;
        }

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
            SwapChainSupportDetails details;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, vkSurface, &details.capabilities);

            uint32_t formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, nullptr);

            if (formatCount != 0) {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, vkSurface, &formatCount, details.formats.data());
            }

            uint32_t presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, nullptr);

            if (presentModeCount != 0) {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, vkSurface, &presentModeCount, details.presentModes.data());
            }

            return details;
        }

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {

            for (const auto& availableFormat : availableFormats) {
                if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                    return availableFormat;
                }
            }

            return availableFormats[0];
        }

        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
            for (VkPresentModeKHR mode : availablePresentModes)
            {
                if(mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                    return mode;
                }
            }
            
            return VK_PRESENT_MODE_FIFO_KHR;
        }

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
            //honestly ive lost the plot here
            if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return capabilities.currentExtent;
            } else {

                int width, height;
                glfwGetFramebufferSize(window, &width, &height);


                VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                };

                actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
                actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

                return actualExtent;
            }
        }

        void createSwapChain() {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

            VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

            uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1; //we want one more than the min for performance
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) { //lets not go over
                imageCount = swapChainSupport.capabilities.maxImageCount; //0 is a special case, it means theres no maxiumum
            }
            submitSemaphores.resize(imageCount);

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = vkSurface;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extent;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
            if(hasSwapChain) {
                createInfo.oldSwapchain = swapChain;
            } else {
                createInfo.oldSwapchain = VK_NULL_HANDLE;
            }

            QueueFamilies families = findQueueFamilies(physicalDevice);
            uint32_t queueFamilyIndices[] = {families.graphicsFamily.value(), families.presentFamily.value()};

            if (families.graphicsFamily != families.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
                createInfo.queueFamilyIndexCount = 0; // Optional
                createInfo.pQueueFamilyIndices = nullptr; // Optional
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform; //no transformation
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //no transperancy in the window itself

            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE; //we dont care about pixels covered by other windows

            if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
                throw std::runtime_error("failed to create swap chain!");
            }
            if(hasSwapChain) vkDestroySwapchainKHR(device,createInfo.oldSwapchain,nullptr);
            hasSwapChain = true;

            vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());


            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;

        }

        void createImageViews() {
            swapChainImageViews.resize(swapChainImages.size());
            for (size_t i = 0; i < swapChainImages.size(); i++) {
                swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat,VK_IMAGE_ASPECT_COLOR_BIT);
            }
        }

        void updateUniformBuffer(uint32_t currentFrame,const Camera& camera) {
            

            SceneDataBufferObject ubo{};

            ubo.view = camera.getViewMatrix();

            ubo.proj = camera.getProjectionMatrix();

            ubo.proj[1][1] *= -1; //flip the y because theres discrepancy
            ubo.viewDir = camera.rotation * vec3(0,0,1);
            
            int width, height;
            glfwGetFramebufferSize(window,&width,&height);
            screenSize = vec2(width,height);
            
            ubo.screen = glm::ortho(0.0f,screenSize.x,0.0f,screenSize.y);

            memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo)); //uniform buffer is always mapped
        }

         void recreateSwapChain() {
            // handle minimization
            int width = 0, height = 0;
            do {
                glfwGetFramebufferSize(window, &width, &height);
                glfwWaitEvents();
            } while  (width == 0 || height == 0);

            screenSize = vec2(width,height);
            
            vkDeviceWaitIdle(device);

            cleanupSwapChain();
            
            
            createSwapChain();
            createImageViews();
            createDepthResources();
            createFrameBuffers();
        }



        bool isDeviceSuitable(VkPhysicalDevice device) {

            // VkPhysicalDeviceProperties deviceProperties;
            // vkGetPhysicalDeviceProperties(device, &deviceProperties);

            // VkPhysicalDeviceFeatures deviceFeatures;
            // vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

            auto families = findQueueFamilies(device);

            bool extensionsSupported = checkDeviceExtensionSupport(device);

            bool swapChainAdequate = false;
            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return families.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }

        bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }

            return requiredExtensions.empty();
        }

        bool checkValidationLayerSupport() {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const char* layerName : validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    std::cout << "validation layer missing: " << layerName << std::endl;
                    return false;
                }
            }

            return true;
        }

        void createSurface() {

            if (glfwCreateWindowSurface(vkInstance, window, nullptr, &vkSurface) != VK_SUCCESS) {
                throw std::runtime_error("failed to create window surface!");
            }

            

        }

        void createLogicalDevice() {
            QueueFamilies families = findQueueFamilies(physicalDevice);

            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
            std::set<uint32_t> uniqueQueueFamilies = {families.graphicsFamily.value(), families.presentFamily.value()};

            float queuePriority = 1.0f; //create all the queues we need for each family
            for (uint32_t queueFamily : uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }
            

            VkPhysicalDeviceBlendOperationAdvancedFeaturesEXT blendOperationFeatures{};
            blendOperationFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BLEND_OPERATION_ADVANCED_FEATURES_EXT;

            VkPhysicalDeviceFeatures2 deviceFeatures2{};
            deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
            deviceFeatures2.features.samplerAnisotropy = VK_TRUE;
            deviceFeatures2.features.fillModeNonSolid = VK_TRUE;
            deviceFeatures2.pNext = &blendOperationFeatures;


            VkPhysicalDeviceVulkan12Features deviceFeatures{};
            deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

            deviceFeatures.pNext = &deviceFeatures2;

            deviceFeatures.scalarBlockLayout = VK_TRUE;

            deviceFeatures.descriptorIndexing = VK_TRUE;
            deviceFeatures.runtimeDescriptorArray = VK_TRUE;
            deviceFeatures.descriptorBindingPartiallyBound = VK_TRUE;
            deviceFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
            deviceFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
            deviceFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;

            deviceFeatures.descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE;
            deviceFeatures.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
            deviceFeatures.descriptorBindingStorageImageUpdateAfterBind = VK_TRUE;
            deviceFeatures.descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE;
            deviceFeatures.shaderUniformBufferArrayNonUniformIndexing = VK_TRUE;
            deviceFeatures.bufferDeviceAddress = true;
            //deviceFeatures.pNext = &descriptorIndexingFeatures;
            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.pNext = &deviceFeatures;

            if(true) { //we have to check for this at some point
                deviceExtensions.push_back("VK_KHR_portability_subset");
                //deviceExtensions.push_back("VK_KHR_get_physical_device_properties2");
            }

            createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
            createInfo.ppEnabledExtensionNames = deviceExtensions.data();

            if (enableValidationLayers) {
                createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
                createInfo.ppEnabledLayerNames = validationLayers.data();
            } else {
                createInfo.enabledLayerCount = 0;
            }

            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }

            vkGetDeviceQueue(device, families.graphicsFamily.value(), 0, &graphicsQueue);
            vkGetDeviceQueue(device, families.presentFamily.value(), 0, &presentQueue);

        }

        void createRenderPass() {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = findDepthFormat();
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            VkSubpassDependency dependency{}; //honestly, i have no idea what this does
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;


            if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw std::runtime_error("failed to create render pass!");
            }

        }

        void createGraphicsPipelineLayout() {


            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 1;
            pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
            
            VkPushConstantRange push_constant;
            //this push constant range starts at the beginning
            push_constant.offset = 0;
            //minimum required size
            push_constant.size = 128;
            //this push constant range is accessible everywhere
            push_constant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &push_constant;

            if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }
            
        }


        VkShaderModule createShaderModule(const std::vector<char>& code) {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shader module!");
            }
            

            return shaderModule;

        }

        void createFrameBuffers() {
            swapChainFramebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                VkImageView attachments[] = {
                    swapChainImageViews[i],
                    depthImageView
                };
                

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 2;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }

        void createCommandPool() {

            QueueFamilies queueFamilies = findQueueFamilies(physicalDevice);

            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();

            if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create command pool!");
            }

        }

        void createCommandBuffer() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = commandPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;
            for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            {
                if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to allocate command buffers!");
                }
            }
        }

        void createSyncObjects() {

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //so the first frame doesn't wait

            for (size_t i = 0; i < swapChainImages.size(); i++)
            {
                if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &submitSemaphores[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create image sync objects!");
                }
            }
            
            for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            {
                if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &acquireSemaphores[i]) != VK_SUCCESS ||
                    vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create frame sync objects!");
                }
            }

        }

        void destroyBuffer(Buffer buffer) {
            if(buffer.allocationIndex != -1) {
                allocatedBuffers[buffer.allocationIndex] = std::nullopt;
            }
            vkDestroyBuffer(device,buffer.buffer,nullptr);
            vkFreeMemory(device,buffer.memory,nullptr);
        }

        void destroyImage(Image image) {
            vkDestroyImage(device,image.image,nullptr);
            vkFreeMemory(device,image.memory,nullptr);
        }

        void destroyTextureResources(TextureResources resources) {
            destroyImage(resources.image);
            vkDestroyImageView(device,resources.view,nullptr);
            vkDestroySampler(device,resources.sampler,nullptr);
        }

        void cleanupSwapChain() {
            for (auto framebuffer : swapChainFramebuffers) {
                vkDestroyFramebuffer(device, framebuffer, nullptr);
            }
            for (auto imageView : swapChainImageViews) {
                vkDestroyImageView(device, imageView, nullptr);
            }
            vkDestroyImageView(device, depthImageView, nullptr);
            destroyImage(depthImage);
        }

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

            for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                    return i;
                }
            }

            throw std::runtime_error("failed to find suitable memory type!");

        }

        Buffer createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkMemoryAllocateFlags memoryAllocateFlags = 0) {

            Buffer buffer;

            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = usage;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer.buffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to create buffer!");
            }

            VkMemoryRequirements memRequirements;
            vkGetBufferMemoryRequirements(device, buffer.buffer, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            if(memoryAllocateFlags != 0) {
                VkMemoryAllocateFlagsInfoKHR flagsInfo{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR};
                flagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
                flagsInfo.pNext = VK_NULL_HANDLE;
                allocInfo.pNext = &flagsInfo;
            }
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &buffer.memory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate buffer memory!");
            }

            vkBindBufferMemory(device, buffer.buffer, buffer.memory, 0);

            return buffer;

        }

        Buffer createManagedBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,VkMemoryAllocateFlags memoryAllocateFlags = 0) {
            Buffer buffer = createBuffer(size,usage,properties,memoryAllocateFlags);
            buffer.allocationIndex = totalAllocatedBuffersCount;
            totalAllocatedBuffersCount++;
            allocatedBuffers.push_back(std::optional(buffer));
            return buffer;
        }

        void copyBuffer(Buffer srcBuffer, Buffer dstBuffer, VkDeviceSize size) {
            //grab a new command buffer
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0; // Optional
            copyRegion.dstOffset = 0; // Optional
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer.buffer, dstBuffer.buffer, 1, &copyRegion);

            endSingleTimeCommands(commandBuffer);
        }

        void createDescriptorSetLayout() {

            VkDescriptorSetLayoutBinding uboLayoutBinding{};
            uboLayoutBinding.binding = 0;
            uboLayoutBinding.descriptorCount = DESCRIPTOR_COUNT;
            uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT; //where its being used
            uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

            VkDescriptorSetLayoutBinding samplerLayoutBinding{};
            samplerLayoutBinding.binding = 1;
            samplerLayoutBinding.descriptorCount = DESCRIPTOR_COUNT;
            samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //this is gonna be used in the fragment shader
            samplerLayoutBinding.pImmutableSamplers = nullptr; // Optional

            VkDescriptorSetLayoutBindingFlagsCreateInfo descriptorBindingFlagsCreateInfo{}; //for bindless
            descriptorBindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
            std::vector<VkDescriptorBindingFlags> bindingFlags = {
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
            };
            descriptorBindingFlagsCreateInfo.bindingCount =  static_cast<uint32_t>(bindingFlags.size());
            descriptorBindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

            std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
            layoutInfo.pBindings = bindings.data();
            layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
            layoutInfo.pNext = &descriptorBindingFlagsCreateInfo;

            if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

        }

        void createDescriptorPool() {

            std::array<VkDescriptorPoolSize, 2> poolSizes{};
            poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSizes[0].descriptorCount = static_cast<uint32_t>(DESCRIPTOR_COUNT);
            poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSizes[1].descriptorCount = static_cast<uint32_t>(DESCRIPTOR_COUNT);

            VkDescriptorPoolCreateInfo poolInfo{};
            poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
            poolInfo.pPoolSizes = poolSizes.data();
            poolInfo.maxSets = static_cast<uint32_t>(1);
            poolInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT; //allows for having huge layouts for bindless

            if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor pool!");
            }

        }

        void createDescriptorSets() {

            std::vector<VkDescriptorSetLayout> layouts(FRAMES_IN_FLIGHT, descriptorSetLayout); //initialize an array of layouts thats this one layout
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = descriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = layouts.data();

            if (vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets!");
            }
            updateDescriptorSet();
        }

        void updateDescriptorSet() {

            std::vector<VkWriteDescriptorSet> descriptorWrites{};

            VkDescriptorBufferInfo bufferInfo[FRAMES_IN_FLIGHT+1];

            for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++)
            {

                bufferInfo[i] = VkDescriptorBufferInfo{};
                bufferInfo[i].buffer = uniformBuffers[i].buffer;
                bufferInfo[i].offset = 0;
                bufferInfo[i].range = sizeof(SceneDataBufferObject);


                VkWriteDescriptorSet cameraObjectWrite{};
                cameraObjectWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                cameraObjectWrite.dstSet = descriptorSet;
                cameraObjectWrite.dstBinding = 0;
                cameraObjectWrite.dstArrayElement = i;
                cameraObjectWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                cameraObjectWrite.descriptorCount = 1;
                cameraObjectWrite.pBufferInfo = &bufferInfo[i];

                descriptorWrites.push_back(cameraObjectWrite);

            }

            std::vector<VkDescriptorImageInfo> descriptorImageInfos;
            descriptorImageInfos.resize(textures.size());

            for (size_t i = 0; i < textures.size(); i++)
            {
            
                descriptorImageInfos[i] = VkDescriptorImageInfo{};
                descriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptorImageInfos[i].imageView = textures[i].view;
                descriptorImageInfos[i].sampler = textures[i].sampler;

                VkWriteDescriptorSet textureWrite{};
                textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                textureWrite.dstSet = descriptorSet;
                textureWrite.dstBinding = 1;
                textureWrite.dstArrayElement = i;
                textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                textureWrite.descriptorCount = 1;
                textureWrite.pImageInfo = &descriptorImageInfos[i];

                descriptorWrites.push_back(textureWrite);
            }
            
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

        void createUniformBuffers() {
            VkDeviceSize bufferSize = sizeof(SceneDataBufferObject);

            for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
                uniformBuffers[i] = createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                VkDebugUtilsObjectNameInfoEXT nameInfo{};
                nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
                nameInfo.objectHandle = reinterpret_cast<uint64_t>(uniformBuffers[i].buffer);
                nameInfo.objectType = VK_OBJECT_TYPE_BUFFER;
                string objectName = std::format("scene data {}",i);
                nameInfo.pObjectName = objectName.c_str();
                vkSetDebugUtilsObjectNameEXT(device,&nameInfo);

                vkMapMemory(device, uniformBuffers[i].memory, 0, bufferSize, 0, &uniformBuffersMapped[i]);
            }
        }

        Image createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            Image image;

            if (vkCreateImage(device, &imageInfo, nullptr, &image.image) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(device, image.image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(device, &allocInfo, nullptr, &image.memory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(device, image.image, image.memory, 0);

            return image;
        }

        Image createTextureImage(int texWidth, int texHeight, int texChannels,void* pixelData) {

            //image library stuff
            VkDeviceSize imageSize = texWidth * texHeight * 4;

            if (!pixelData) {
                throw std::runtime_error("failed to load texture image!");
            }
            
            // create the staging buffer
            
            Buffer stagingBuffer = createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            
            // image data into the staging buffer
            void* data;
            vkMapMemory(device, stagingBuffer.memory, 0, imageSize, 0, &data);
                memcpy(data, pixelData, static_cast<size_t>(imageSize));
            vkUnmapMemory(device, stagingBuffer.memory);

            Image image = createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            
            //copy the pixel data into the image
            transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            copyBufferToImage(stagingBuffer.buffer, image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

            //transition it back into a format(?) usage by the shader
            transitionImageLayout(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            destroyBuffer(stagingBuffer);

            return image;
        }

        VkImageView createImageView(Image image, VkFormat format, VkImageAspectFlags aspectFlags) {
            return createImageView(image.image,format,aspectFlags);
        }

        VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = aspectFlags;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkImageView imageView;
            if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image view!");
            }

            return imageView;
        }

        VkImageView createTextureImageView(Image image) {
            return createImageView(image, VK_FORMAT_R8G8B8A8_SRGB,VK_IMAGE_ASPECT_COLOR_BIT);
        }

        VkSampler createTextureSampler(Image image) {
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = VK_TRUE;

            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; //we have to query the device for this info 
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE; //if true, it would use pixel coordiantes 
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;

            VkSampler sampler;

            if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
                throw std::runtime_error("failed to create texture sampler!");
            }

            return sampler;


        }

        void transitionImageLayout(Image image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;

            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            barrier.image = image.image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

                if (hasStencilComponent(format)) {
                    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
                }
            } else {
                barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            }

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            } else {
                throw std::invalid_argument("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );


            endSingleTimeCommands(commandBuffer);
        }

        void copyBufferToImage(VkBuffer buffer, Image image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = beginSingleTimeCommands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;

            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;

            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(
                commandBuffer,
                buffer,
                image.image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &region
            );

            endSingleTimeCommands(commandBuffer);
        }

        void createDepthResources() {
            VkFormat depthFormat = findDepthFormat();
            depthImage = createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
            depthImageView = createImageView(depthImage, depthFormat,VK_IMAGE_ASPECT_DEPTH_BIT);

            transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
            
        }

        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {

            for (VkFormat format : candidates) {
                VkFormatProperties props;
                vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

                if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                    return format;
                } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                    return format;
                }
            }

            throw std::runtime_error("failed to find supported format!");

        }

        bool hasStencilComponent(VkFormat format) {
            return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
        }

        VkFormat findDepthFormat() {
            return findSupportedFormat(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
            );
        }


        VkCommandBuffer beginSingleTimeCommands() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;
        }

        void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue);

            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        void initImGui() {
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000;
            pool_info.poolSizeCount = std::size(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;

            VkDescriptorPool imguiPool;
            if(vkCreateDescriptorPool(device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS) {
                throw new std::runtime_error("Could not create imgui descriptor pool");
            }

            //this initializes the core structures of imgui
            ImGui::CreateContext();

            //this initializes imgui for SDL
            ImGui_ImplGlfw_InitForVulkan(window,true);

            //this initializes imgui for Vulkan
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = vkInstance;
            init_info.PhysicalDevice = physicalDevice;
            init_info.Device = device;
            init_info.Queue = graphicsQueue;
            init_info.DescriptorPool = imguiPool;
            init_info.MinImageCount = 3;
            init_info.ImageCount = 3;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
            init_info.Allocator = nullptr;
            init_info.RenderPass = renderPass;

            
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(init_info.PhysicalDevice, &properties);

            ImGui_ImplVulkan_Init(&init_info);


        }

        
        

};