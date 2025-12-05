#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "graphics/camera.hpp"
#include "graphics/mesh-data.hpp"
#include "graphics/color.hpp"
#include <glm/glm.hpp>

#include <fstream>

using std::vector, std::string, glm::vec3;

enum InfoPriority{
    HIGH,
    MEDIUM,
    LOW
};

class Debug {

    struct DebugVertex {
        glm::vec3 pos;
        DebugVertex() {}

        DebugVertex(glm::vec3 pos) : pos(pos) {}

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(DebugVertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(DebugVertex, pos);

            return attributeDescriptions;
        }
    };

    struct DebugMaterialData {
        vec3 color;
        DebugMaterialData () {}
        DebugMaterialData (vec3 color) : color(color) {}
    };

    struct Point {
        vec3 position;
        Color color;
        float time;
        Point(vec3 position,Color color = Color::green,float time = 0) : position(position), color(color), time(time) {}
    };

    struct Line {
        vec3 a;
        vec3 b;
        Color color;
        float time;
        Line(vec3 a,vec3 b,Color color = Color::green,float time = 0) : a(a), b(b), color(color), time(time) {}
    };
    struct Cube {
        vec3 origin;
        vec3 size;
        Color color;
        float time;
        Cube(vec3 origin,vec3 size,Color color = Color::green,float time = 0) : origin(origin), size(size), color(color), time(time) {}
    };

    private:
        static std::unique_ptr<Debug> _instance;

        vector<string> trace;


        vector<Point> pointsToDraw;
        vector<Line> linesToDraw;
        vector<Cube> cubesToDraw;

        // std::unique_ptr<Model> _quadModel;
        // std::unique_ptr<Shader> _shader;
        float pointSize = 0.05;
        
        std::ofstream logFile;

        bool readyToRender = false;
        Material wireFrameMaterial = Material::none;
        Material solidMaterial = Material::none;

        MeshBuffer quad;
        MeshBuffer line;
        //MeshBuffer cube;

        

    

    public:

        Debug() {
            logFile = std::ofstream("log.txt", std::ofstream::trunc);
        }

        static void loadRenderResources(Vulkan& vulkan) {
            Debug& debug = getInstance();

            PipelineOptions pOptions;
            pOptions.polygonMode = VK_POLYGON_MODE_LINE;
            pOptions.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            auto wireFramePipeline = vulkan.createManagedPipeline<DebugVertex>("shaders/compiled/debug_vert.spv","shaders/compiled/debug_frag.spv",pOptions);
            debug.wireFrameMaterial = vulkan.createMaterial(wireFramePipeline,DebugMaterialData(vec3(0,1,0)));

            pOptions = PipelineOptions();
            auto solidPipeline = vulkan.createManagedPipeline<DebugVertex>("shaders/compiled/debug_vert.spv","shaders/compiled/debug_frag.spv",pOptions);
            debug.solidMaterial = vulkan.createMaterial(solidPipeline,DebugMaterialData(vec3(0,1,0)));
            
            //debug.solidPipeline = vulkan.createManagedPipeline<DebugVertex>("shaders/compiled/debug_vert.spv","shaders/compiled/debug_frag.spv");

            // line
            MeshData<DebugVertex> data;
            data.vertices = std::vector<DebugVertex> {
                DebugVertex(vec3(0,0,0)),
                DebugVertex(vec3(1,1,1)),
            };
            data.indices = std::vector<uint16_t> {
                0,1
            };
            debug.line = vulkan.createMeshBuffers<DebugVertex>(data.vertices,data.indices);


            data.vertices = std::vector<DebugVertex> {
                DebugVertex(vec3(0,0,0)),
                DebugVertex(vec3(0,1,0)),
                DebugVertex(vec3(1,0,0)),
                DebugVertex(vec3(1,1,0))
            };
            data.indices = std::vector<uint16_t> {
                0,1,2,
                1,2,3
            };
            debug.quad = vulkan.createMeshBuffers<DebugVertex>(data.vertices,data.indices);

            debug.readyToRender = true;
        }

        static Debug& getInstance() {
            if(_instance == nullptr) {
                _instance = std::make_unique<Debug>();
            }
            return *_instance.get();
        }

        // static Model* getQuadModel() {
        //     Debug* instance = getInstance();
        //     if(instance->_quadModel == nullptr) {
        //         instance->_quadModel = std::make_unique<Model>();
        //         instance->_quadModel->loadQuad();
        //     }
        //     return instance->_quadModel.get();
        // }

        static string getVertShaderSource() {
            return R"(
                #version 410
                layout (location = 0) in vec3 aPos;
                layout (location = 1) in vec3 aNormal;
                layout (location = 2) in vec2 aTexCoord;

                out vec3 normal;
                out vec2 texCoord;

                uniform mat4 view;
                uniform mat4 model;
                uniform mat4 projection;
                uniform vec4 color;

                void main()
                {
                    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
                    texCoord = aTexCoord;
                    normal = mat3(transpose(inverse(model))) * aNormal;
                }
            )";
        }

        static string getFragShaderSource() {
            return R"(
                #version 410

                in vec2 texCoord;
                in vec3 normal;

                out vec4 FragColor;

                uniform vec4 color;

                void main()
                {
                    FragColor = color;
                }
            )";
        }

        // static Shader* getShader() {
        //     Debug* instance = getInstance();
        //     if(instance->_shader == nullptr) {
        //         instance->_shader = std::make_unique<Shader>();
        //         instance->_shader->loadFromMemory(getVertShaderSource().c_str(),getFragShaderSource().c_str());
        //     }
        //     return instance->_shader.get();
        // }

        
        
        static void addTrace(string t) {
            vector<string>& trace = getInstance().trace;
            trace.push_back(t);
        }

        static void subtractTrace() {
            vector<string>& trace = getInstance().trace;
            if(trace.size() > 0) {
                trace.pop_back();
            }
        }

        static string traceString() {
            vector<string>& trace = getInstance().trace;
            string fullTrace = " (";
            bool first = true;
            for(string tracePart : trace) {
                if(!first) fullTrace += ".";
                fullTrace += tracePart;
                first = false;
            }
            fullTrace += ")";
            return fullTrace;
        }

        static void warn(string string) {
            Debug& instance = getInstance();
            std::cout << "[WARNING] " << string << traceString() << std::endl;
            instance.logFile << "[WARNING] " << string << traceString() << std::endl;
        }

        static void info(string string,InfoPriority priority) {
            Debug& instance = getInstance();
            std::cout << "[INFO] " << string << traceString() << std::endl;
            instance.logFile << "[INFO] " << string << traceString() << std::endl;
        }

        //set the draw
        static void drawPoint(vec3 position,Color color = Color::green) {
            Debug& instance = getInstance();
            instance.pointsToDraw.push_back(Point(position,color));
        }

        static void drawLine(vec3 a,vec3 b,Color color = Color::green) {
            Debug& instance = getInstance();
            instance.linesToDraw.push_back(Line(a,b,color));
        }

        static void drawRay(vec3 origin,vec3 direction,Color color = Color::green) {
            Debug& instance = getInstance();
            instance.linesToDraw.push_back(Line(origin,origin + direction,color));
        }

        static void drawCube(vec3 origin,vec3 size,Color color = Color::green) {
            Debug& instance = getInstance();
            instance.cubesToDraw.push_back(Cube(origin,size,color));
        }

        //actually render the shapes
        static void addRenderables(Vulkan& vulkan) {

            Debug& instance = getInstance();
            if(!instance.readyToRender) {
                Debug::warn("Debug shapes not ready to render");
                return;
            }

            // for (auto& p : instance->pointsToDraw)
            // {
            //     Model* model = instance->getQuadModel();
            //     Shader& shader = *instance->getShader();
            //     shader.use();
            //     shader.setColor("color",p.color);
            //     glm::mat4 modelMatrix = glm::mat4(1.0f);
            //     modelMatrix = glm::translate(modelMatrix,p.position);
            //     modelMatrix = glm::scale(modelMatrix,vec3(0.1,0.1,0.1));
            //     modelMatrix *= glm::inverse(camera.getViewRotationMatrix());
            //     model->render(camera.getViewMatrix(),modelMatrix,camera.getProjectionMatrix(),shader);
            // }

            for (auto& p : instance.pointsToDraw)
            {
                auto mat = glm::mat4(1.0f);
                mat = glm::translate(mat,p.position);
                mat = glm::scale(mat,vec3(0.1));
                vulkan.addMesh(instance.quad,instance.solidMaterial,mat);
            }
    

            for (auto& l : instance.linesToDraw)
            {
                auto mat = glm::mat4(1.0f);
                mat = glm::translate(mat,l.a);
                mat = glm::scale(mat,l.b-l.a);
                vulkan.addMesh(instance.line,instance.wireFrameMaterial,mat);
            }
    

            // Model cubeModel;
            // cubeModel.loadFromFile("models/block.obj");
            // for (auto& c : instance->cubesToDraw)
            // {
                
            //     Shader& shader = *instance->getShader();
            //     shader.use();
            //     shader.setColor("color",c.color);
            //     cubeModel.renderMode = Model::RenderMode::Wireframe;
            //     glm::mat4 modelMatrix = glm::mat4(1.0f);
            //     modelMatrix = glm::translate(modelMatrix,c.origin);
            //     modelMatrix = glm::scale(modelMatrix,c.size);
            //     cubeModel.render(camera.getViewMatrix(),modelMatrix,camera.getProjectionMatrix(),shader);
            // }
        
            

        }

        static void clearDebugShapes() {
            Debug& instance = getInstance();
            instance.linesToDraw.clear();
            instance.pointsToDraw.clear();
            instance.cubesToDraw.clear();
        }

};

std::unique_ptr<Debug> Debug::_instance = nullptr;