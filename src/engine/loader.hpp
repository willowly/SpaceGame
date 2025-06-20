#pragma 
#include "registry.hpp"
#include "graphics/model.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"

using std::string,std::vector;

class Loader {

    public:
        void loadAll(Registry& registry) {
            loadModels(registry);
            loadTextures(registry);
            loadShaders(registry);
        }

        void loadModels(Registry& registry) {
            std::cout << "Loading Models" << std::endl;
            loadModelsFromDir(registry,"models");
        }

        void loadModelsFromDir(Registry& registry,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadModelsFromDir(registry,entry.path());
                    continue;
                }

                if(entry.path().extension() != ".obj") continue;
                
                string name = p.stem().string();
                if(registry.textures.count(name) == 0) {
                    registry.models.emplace(name,Model());
                } else {
                    registry.models[name] = Model(); //replace since we can't update static models :shrug:
                }
                Model& model = registry.models.at(name);
                model.loadFromFile(entry.path());

            }
        }

        void loadTextures(Registry& registry) {
            std::cout << "Loading Textures" << std::endl;
            loadTexturesFromDir(registry,"textures");
            if(!registry.textures.contains("error")) {
                std::cout << "[WARNING] no fallback/error texture!" << std::endl;
            }
        }

        void loadTexturesFromDir(Registry& registry,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadModelsFromDir(registry,entry.path());
                    continue;
                }
                
                string extension = entry.path().extension();
                if(extension != ".png" && extension != ".jpg" && extension != ".jpeg") continue;
                
                string name = p.stem().string();
                if(registry.textures.count(name) == 0) {
                    registry.textures.emplace(name,Texture());
                }
                Texture& texture = registry.textures.at(name);
                if(extension == ".png") {
                    texture.loadFromFile(entry.path(),Texture::Format::RGBA);
                }
                if(extension == ".jpg" || extension == ".jpeg") {
                    texture.loadFromFile(entry.path(),Texture::Format::RGB);
                }

            }
        }

        void loadShaders(Registry& registry) {
            std::cout << "Loading Shaders" << std::endl;
            registry.litShader.loadFromFiles("shaders/lit.vert","shaders/lit.frag");
        }

};