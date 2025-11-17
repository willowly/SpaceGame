#pragma once
#include "registry.hpp"
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include <filesystem>

#include "api/api-all.hpp"

#include "debug.hpp"

using std::string,std::vector;

class Loader {

    public:

        

        void loadAll(Registry& registry,sol::state& lua,Vulkan* vulkan) {
            std::cout << "Current loader path is: " << std::filesystem::current_path() << '\n';
            loadModels(registry,vulkan);
            loadTextures(registry,vulkan);
            loadShaders(registry,vulkan);
            runLoadScript(registry,vulkan,lua);
        }

        void loadModels(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("models");
            std::cout << "Loading Models" << std::endl;
            
            loadModelsFromDir(registry,vulkan,"models");
            Debug::subtractTrace();
            
        }

        void loadModelsFromDir(Registry& registry,Vulkan* vulkan,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadModelsFromDir(registry,vulkan,entry.path().string());
                    continue;
                }

                if(entry.path().extension() != ".obj") continue;
                
                string name = p.stem().string();
                registry.addModel(name);
                Mesh<Vertex>* model = registry.getModel(name);
                model->loadFromFile(entry.path().string());
                model->createBuffers(vulkan);
                Debug::info("Loaded Model \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadTextures(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("textures");
            std::cout << "Loading Textures" << std::endl;
            loadTexturesFromDir(registry,vulkan,"textures");
            if(!registry.hasTexture("error")) {
                Debug::warn("[WARNING] no fallback/error texture!");
            }
            Debug::subtractTrace();
        }

        void loadTexturesFromDir(Registry& registry,Vulkan* vulkan,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadTexturesFromDir(registry,vulkan,entry.path().string());
                    continue;
                }
                
                string extension = entry.path().extension().string();
                if(extension != ".png" /*&& extension != ".jpg" && extension != ".jpeg"*/) continue;
                
                string name = p.stem().string();
                
                if(extension == ".png") {
                    TextureID id = vulkan->loadTextureFile(entry.path().string());
                    registry.addTexture(name,id);
                }
                // if(extension == ".jpg" || extension == ".jpeg") {
                //     texture->loadFromFile(entry.path().string(),Texture::Format::RGB);
                // }

                Debug::info("Loaded Texture \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadShaders(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("shaders");
            std::cout << "Loading Shaders" << std::endl;
            registry.litShader = vulkan->createManagedPipeline<Vertex>("shaders/compiled/lit_vert.spv","shaders/compiled/lit_frag.spv");
            // registry.textShader.loadFromFiles("shaders/text.vert","shaders/text.frag");
            // registry.uiShader.loadFromFiles("shaders/ui.vert","shaders/ui.frag");
            Debug::subtractTrace();
        }

        void runLoadScript(Registry& registry,Vulkan* vulkan,sol::state& lua) {
            std::cout << "Running load.lua" << std::endl;
            lua["textures"] = API::TextureRegistry(registry);
            // lua["shaders"] = API::ShaderRegistry(registry);
            lua["materials"] = API::MaterialRegistry(registry,vulkan);
            lua["actors"] = API::ActorRegistry(registry);
            lua["blocks"] = API::BlockRegistry(registry);
            lua.do_file("scripts/load.lua");
        }

        static string vertCodePath(string name) {
            return "shaders/compiled/" + name + "_vert.spv";
        }

        static string fragCodePath(string name) {
            return "shaders/compiled/" + name + "_frag.spv";
        }

};