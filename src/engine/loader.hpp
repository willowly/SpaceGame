#pragma once
#include "registry.hpp"
#include "graphics/model.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include <filesystem>

#include "api/api-all.hpp"

#include "debug.hpp"

using std::string,std::vector;

class Loader {

    public:

        

        void loadAll(Registry& registry,sol::state& lua) {
            std::cout << "Current loader path is: " << std::filesystem::current_path() << '\n';
            loadModels(registry);
            loadTextures(registry);
            loadShaders(registry);
            runLoadScript(registry,lua);
        }

        void loadModels(Registry& registry) {
            Debug::addTrace("models");
            std::cout << "Loading Models" << std::endl;
            
            loadModelsFromDir(registry,"models");
            Debug::subtractTrace();
            
        }

        void loadModelsFromDir(Registry& registry,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadModelsFromDir(registry,entry.path().string());
                    continue;
                }

                if(entry.path().extension() != ".obj") continue;
                
                string name = p.stem().string();
                registry.addModel(name);
                Model* model = registry.getModel(name);
                model->loadFromFile(entry.path().string());
                Debug::info("Loaded Model \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadTextures(Registry& registry) {
            Debug::addTrace("textures");
            std::cout << "Loading Textures" << std::endl;
            loadTexturesFromDir(registry,"textures");
            if(!registry.hasTexture("error")) {
                Debug::warn("[WARNING] no fallback/error texture!");
            }
            Debug::subtractTrace();
        }

        void loadTexturesFromDir(Registry& registry,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadTexturesFromDir(registry,entry.path().string());
                    continue;
                }
                
                string extension = entry.path().extension().string();
                if(extension != ".png" && extension != ".jpg" && extension != ".jpeg") continue;
                
                string name = p.stem().string();
                if(!registry.hasTexture(name)) {
                    registry.addTexture(name);
                }
                Texture* texture = registry.getTexture(name);
                if(extension == ".png") {
                    texture->loadFromFile(entry.path().string(),Texture::Format::RGBA);
                }
                if(extension == ".jpg" || extension == ".jpeg") {
                    texture->loadFromFile(entry.path().string(),Texture::Format::RGB);
                }

                Debug::info("Loaded Texture \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadShaders(Registry& registry) {
            Debug::addTrace("shaders");
            std::cout << "Loading Shaders" << std::endl;
            registry.litShader.loadFromFiles("shaders/lit.vert","shaders/lit.frag");
            registry.textShader.loadFromFiles("shaders/text.vert","shaders/text.frag");
            registry.uiShader.loadFromFiles("shaders/ui.vert","shaders/ui.frag");
            Debug::subtractTrace();
        }

        void runLoadScript(Registry& registry,sol::state& lua) {
            std::cout << "Running load.lua" << std::endl;
            lua["textures"] = API::TextureRegistry(registry);
            lua["shaders"] = API::ShaderRegistry(registry);
            lua["materials"] = API::MaterialRegistry(registry);
            lua["actors"] = API::ActorRegistry(registry);
            lua["blocks"] = API::BlockRegistry(registry);
            lua.do_file("scripts/load.lua");
        }

};