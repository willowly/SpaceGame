#pragma once
#include <vector>
#include <string>
#include <iostream>

#include "graphics/camera.hpp"
#include "graphics/model.hpp"
#include "graphics/color.hpp"
#include <glm/glm.hpp>

#include <fstream>

using std::vector, std::string;

enum InfoPriority{
    HIGH,
    MEDIUM,
    LOW
};

class Debug {

    struct Point {
        vec3 position;
        Color color;
        float time;
        Point(vec3 position,Color color = Color::blue,float time = 0) : position(position), color(color), time(time) {}
    };

    struct Line {
        vec3 a;
        vec3 b;
        Color color;
        float time;
        Line(vec3 a,vec3 b,Color color = Color::blue,float time = 0) : a(a), b(b), color(color), time(time) {}
    };
    struct Cube {
        vec3 origin;
        vec3 size;
        Color color;
        float time;
        Cube(vec3 origin,vec3 size,Color color = Color::blue,float time = 0) : origin(origin), size(size), color(color), time(time) {}
    };

    private:
        static std::unique_ptr<Debug> _instance;

        vector<string> trace;


        vector<Point> pointsToDraw;
        vector<Line> linesToDraw;
        vector<Cube> cubesToDraw;

        std::unique_ptr<Model> _quadModel;
        std::unique_ptr<Shader> _shader;
        float pointSize = 0.05;
        
        std::ofstream logFile;

        

    

    public:

        Debug() {
            logFile = std::ofstream("log.txt", std::ofstream::trunc);
        }

        static Debug* getInstance() {
            if(_instance == nullptr) {
                _instance = std::make_unique<Debug>();
            }
            return _instance.get();
        }

        static Model* getQuadModel() {
            Debug* instance = getInstance();
            if(instance->_quadModel == nullptr) {
                instance->_quadModel = std::make_unique<Model>();
                instance->_quadModel->loadQuad();
            }
            return instance->_quadModel.get();
        }

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

        static Shader* getShader() {
            Debug* instance = getInstance();
            if(instance->_shader == nullptr) {
                instance->_shader = std::make_unique<Shader>();
                instance->_shader->loadFromMemory(getVertShaderSource().c_str(),getFragShaderSource().c_str());
            }
            return instance->_shader.get();
        }

        
        
        static void addTrace(string t) {
            vector<string>& trace = getInstance()->trace;
            trace.push_back(t);
        }

        static void subtractTrace() {
            vector<string>& trace = getInstance()->trace;
            if(trace.size() > 0) {
                trace.pop_back();
            }
        }

        static string traceString() {
            vector<string>& trace = getInstance()->trace;
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
            Debug* instance = getInstance();
            std::cout << "[WARNING] " << string << traceString() << std::endl;
            instance->logFile << "[WARNING] " << string << traceString() << std::endl;
        }

        static void info(string string,InfoPriority priority) {
            Debug* instance = getInstance();
            std::cout << "[INFO] " << string << traceString() << std::endl;
            instance->logFile << "[INFO] " << string << traceString() << std::endl;
        }

        //set the draw
        static void drawPoint(vec3 position,Color color = Color::blue) {
            Debug* instance = getInstance();
            instance->pointsToDraw.push_back(Point(position,color));
        }

        static void drawLine(vec3 a,vec3 b,Color color = Color::blue) {
            Debug* instance = getInstance();
            instance->linesToDraw.push_back(Line(a,b,color));
        }

        static void drawRay(vec3 origin,vec3 direction,Color color = Color::blue) {
            Debug* instance = getInstance();
            instance->linesToDraw.push_back(Line(origin,origin + direction,color));
        }

        static void drawCube(vec3 origin,vec3 size,Color color = Color::blue) {
            Debug* instance = getInstance();
            instance->cubesToDraw.push_back(Cube(origin,size,color));
        }

        //actually render the shapes
        static void renderDebugShapes(Camera camera) {

            Debug* instance = getInstance();

            for (auto& p : instance->pointsToDraw)
            {
                Model* model = instance->getQuadModel();
                Shader& shader = *instance->getShader();
                shader.use();
                shader.setColor("color",p.color);
                glm::mat4 modelMatrix = glm::mat4(1.0f);
                modelMatrix = glm::translate(modelMatrix,p.position);
                modelMatrix = glm::scale(modelMatrix,vec3(0.1,0.1,0.1));
                modelMatrix *= glm::inverse(camera.getViewRotationMatrix());
                model->render(camera.getViewMatrix(),modelMatrix,camera.getProjectionMatrix(),shader);
            }
        
            
            instance->pointsToDraw.clear();

            for (auto& l : instance->linesToDraw)
            {
                Model model;
                model.loadLine(l.a,l.b);
                Shader& shader = *instance->getShader();
                shader.use();
                shader.setColor("color",l.color);
                model.renderMode = Model::RenderMode::Wireframe;
                model.render(camera.getViewMatrix(),glm::mat4(1.0f),camera.getProjectionMatrix(),shader);
            }
        
            instance->linesToDraw.clear();

            Model cubeModel;
            cubeModel.loadFromFile("models/block.obj");
            for (auto& l : instance->cubesToDraw)
            {
                
                Shader& shader = *instance->getShader();
                shader.use();
                shader.setColor("color",l.color);
                cubeModel.renderMode = Model::RenderMode::Wireframe;
                cubeModel.render(camera.getViewMatrix(),glm::mat4(1.0f),camera.getProjectionMatrix(),shader);
            }
        
            instance->linesToDraw.clear();

        }

};

std::unique_ptr<Debug> Debug::_instance = nullptr;