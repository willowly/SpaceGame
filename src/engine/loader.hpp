#pragma once
#include "registry.hpp"
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include "api/asset-serializer.hpp"
#include <filesystem>

#include "api/api-all.hpp"

#include "api/type-info-all.hpp"

#include "debug.hpp"

using std::string,std::vector;

class Loader {

    public:

        inline const static string DEFAULT_CONSTRUCTION_MATERIAL_KEY = "default_construction";
        inline const static string DEFAULT_TERRAIN_MATERIAL_KEY = "default_terrain";

        void loadAll(Registry& registry,sol::state& lua,Vulkan* vulkan) {
            copyAssets();
            loadModels(registry,vulkan);
            loadTextures(registry,vulkan);
            loadShaders(registry,vulkan);
            loadDefaultMaterials(registry,vulkan);
            loadFonts(registry,vulkan);
            loadTypeInfo(registry);
            loadAssetFileStubs(registry,vulkan);
            runLoadScript(registry,vulkan,lua);
            loadAssetFileProperties(registry,vulkan);
        }
        
        void copyAssets() {
            string path = "..\\..\\assets";
            auto source = std::filesystem::path(path);
            auto dest = std::filesystem::current_path().append("assets");
            if(std::filesystem::exists(path)) {
                copyAssetsFromDir(source,dest);
                deleteMissingAssetsFromDir(source,dest);
            }
        }

        void deleteMissingAssetsFromDir(std::filesystem::path sourcePath,std::filesystem::path destPath) {
            for (const auto & entry : std::filesystem::directory_iterator(destPath)) {
                auto entryDest = entry.path();
                auto entrySource = sourcePath;
                entrySource.append(entry.path().filename().string());

                if(!std::filesystem::exists(entrySource) ) {
                    std::filesystem::remove(entryDest);
                    Debug::info("deleted " + entryDest.string(),InfoPriority::MEDIUM);
                    continue;
                }

                if(entry.is_directory()) {
                    deleteMissingAssetsFromDir(entrySource,entryDest);
                    continue;
                }
            }
        }

        void copyAssetsFromDir(std::filesystem::path sourcePath,std::filesystem::path destPath) {
            
            if(!std::filesystem::exists(destPath)) {
                std::filesystem::create_directory(destPath);
            }
            for (const auto & entry : std::filesystem::directory_iterator(sourcePath)) {
                auto entrySource = entry.path();
                auto entryDest = destPath;
                entryDest.append(entry.path().filename().string());

                if(entry.is_directory()) {
                    copyAssetsFromDir(entrySource,entryDest);
                    continue;
                }

                std::filesystem::copy_file(entrySource,entryDest,std::filesystem::copy_options::overwrite_existing);

            }
        }


        void loadModels(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("models");
            std::cout << "Loading Models" << std::endl;
            
            loadModelsFromDir(registry,vulkan,"assets/models");
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
                model->updateBuffers(vulkan);
                Debug::info("Loaded Model \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadTextures(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("textures");
            std::cout << "Loading Textures" << std::endl;
            vulkan->clearTextures(); // easiest way to do this
            loadTexturesFromDir(registry,vulkan,"assets/textures");
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
                    registry.setTexture(name,id);
                    vulkan->setTextureName(id,name);
                }
                // if(extension == ".jpg" || extension == ".jpeg") {
                //     texture->loadFromFile(entry.path().string(),Texture::Format::RGB);
                // }

                Debug::info("Loaded Texture \"" + name + "\"",InfoPriority::MEDIUM);

            }
        }

        void loadDefaultMaterials(Registry& registry,Vulkan* vulkan) {
            registry.addMaterial(DEFAULT_CONSTRUCTION_MATERIAL_KEY,vulkan->createMaterial<LitMaterialData,ConstructionVertex>("construction",LitMaterialData(registry.getTexture("rock"))));
            registry.addMaterial(DEFAULT_TERRAIN_MATERIAL_KEY,vulkan->createMaterial<LitMaterialData,TerrainVertex>("terrain",LitMaterialData(registry.getTexture("rock"))));
        }

        void loadShaders(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("shaders");
            std::cout << "Loading Shaders" << std::endl;
            registry.litShader = vulkan->createManagedPipeline<Vertex>("shaders/compiled/lit_vert.spv","shaders/compiled/lit_frag.spv");
            // registry.textShader.loadFromFiles("shaders/text.vert","shaders/text.frag");
            // registry.uiShader.loadFromFiles("shaders/ui.vert","shaders/ui.frag");
            Debug::subtractTrace();
        }

        void loadFonts(Registry& registry,Vulkan* vulkan) {
            auto* font = registry.addAny<Font>("default",Font());
            font->texture = registry.getTexture("characters");
            font->start = '0';
            font->charSize = vec2(8,12);
            font->textureSize = vec2(352,12);
            font->characters = "0123456789x./+-=_ ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        }

        void loadAssetFileStubs(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("items");
            loadObjectStubsFromDir<Item>(registry,vulkan,"assets/items");
            loadObjectStubsFromDir<Block>(registry,vulkan,"assets/blocks");
            loadObjectStubsFromDir<MaterialObject>(registry,vulkan,"assets/materials");
            loadObjectStubsFromDir<Recipe>(registry,vulkan,"assets/recipes");
            loadObjectStubsFromDir<Actor>(registry,vulkan,"assets/actors");
            loadObjectStubsFromDir<Widget>(registry,vulkan,"assets/widgets");
            Debug::subtractTrace();
        }

        void loadAssetFileProperties(Registry& registry,Vulkan* vulkan) {
            Debug::addTrace("items");
            loadObjectPropertiesFromDir<Item>(registry,vulkan,"assets/items");
            loadObjectPropertiesFromDir<Block>(registry,vulkan,"assets/blocks");
            loadObjectPropertiesFromDir<MaterialObject>(registry,vulkan,"assets/materials");
            loadObjectPropertiesFromDir<Recipe>(registry,vulkan,"assets/recipes");
            loadObjectPropertiesFromDir<Actor>(registry,vulkan,"assets/actors");
            loadObjectPropertiesFromDir<Widget>(registry,vulkan,"assets/widgets");
            for(auto pair : registry.getObjects<MaterialObject>()) {
                auto mat = pair.second;
                mat->loadMaterial(vulkan);
            }
            Debug::subtractTrace();
        }

        template<typename T>
        void loadObjectStubsFromDir(Registry& registry,Vulkan* vulkan,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadObjectStubsFromDir<T>(registry,vulkan,entry.path().string());
                    continue;
                }
                
                string extension = entry.path().extension().string();
                if(extension != ".asset") continue;
                
                string name = p.stem().string();

                Debug::addTrace(name);
                
                AssetSerializer serializer;
                serializer.registry = &registry;
                serializer.vulkan = vulkan;

                std::ifstream file(entry.path().string());

                auto obj = serializer.deserializeStub<T>(file);

                registry.addObject<T>(name,std::move(obj));

                Debug::info("Loaded Object \"" + name + "\" (" + registry.getTypeName(typeid(T).name()) + ")",InfoPriority::MEDIUM);
                Debug::subtractTrace();
            }
        }

        template<typename T>
        void loadObjectPropertiesFromDir(Registry& registry,Vulkan* vulkan,string path) {
            for (const auto & entry : std::filesystem::directory_iterator(path)) {
                std::filesystem::path p(entry.path());

                if(entry.is_directory()) {
                    loadObjectPropertiesFromDir<T>(registry,vulkan,entry.path().string());
                    continue;
                }
                
                string extension = entry.path().extension().string();
                if(extension != ".asset") continue;
                
                string name = p.stem().string();

                Debug::addTrace(name);
                
                AssetSerializer serializer;
                serializer.registry = &registry;
                serializer.vulkan = vulkan;

                std::ifstream file(entry.path().string());

                auto obj = registry.getPtr<T>(name);

                serializer.deserializeProperties(obj,file);

                Debug::info("Loaded Object Properties \"" + name + "\" (" + registry.getTypeName(typeid(T).name()) + ")",InfoPriority::MEDIUM);
                Debug::subtractTrace();
            }
        }

        void loadTypeInfo(Registry& registry) {
            TypeInfoLoader::loadAll(registry);
        }

        void runLoadScript(Registry& registry,Vulkan* vulkan,sol::state& lua) {
            std::cout << "Running load.lua" << std::endl;
            lua["textures"] = API::TextureRegistry(registry);
            // lua["shaders"] = API::ShaderRegistry(registry);
            lua["materials"] = API::MaterialRegistry(registry,vulkan);
            lua["actors"] = API::ActorRegistry(registry);
            lua["blocks"] = API::BlockRegistry(registry);
            lua["items"] = API::ItemRegistry(registry);
            lua["recipes"] = API::RecipeRegistry(registry);
            lua["widgets"] = API::WidgetRegistry(registry);
            lua.do_file("scripts/load.lua");
        }

};