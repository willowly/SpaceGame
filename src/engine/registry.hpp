#pragma once
#include "graphics/mesh.hpp"
#include "graphics/vulkan.hpp"
#include "actor/actor.hpp"
#include "block/block.hpp"
#include "item/item.hpp"
#include "helper/sprite.hpp"
#include "interface/widget.hpp"
#include "actor/components/particle-effect.hpp"
#include "interface/font.hpp"
#include <map>
#include <memory>

#include "engine/debug.hpp"
#include <algorithm>
#include <type_traits>
#include "api/type-info.hpp"
#include <iterator>

using std::map,std::unique_ptr;

class Registry {

    // id like to do this at some point, unify actors blocks items widgets recipes particles etc into one thing
        // struct Prototype {

        //     virtual ~Prototype() = 0;

        // };

        // template<typename T>
        // struct TypedPrototype {
        //     std::unique_ptr<T> ptr;
        // };

        // map<string,std::unique_ptr<Prototype>> prototypes;

    

    map<string,Mesh<Vertex>> models;
    map<string,TextureID> textures;
    map<string,Sprite> sprites;
    map<string,Material> materials;
    map<string,std::unique_ptr<Object>> materialDataMap;
    
    map<string,unique_ptr<Actor>> actors;
    // map<string,unique_ptr<Block>> blocks;
    // map<string,unique_ptr<Item>> items;
    //map<string,unique_ptr<Widget>> widgets;
    
    map<string,ParticleEffect> particleEffects;

    map<string,TypeInfo> typeInfo;
    map<string,string> typeIdToName;

    map<string,map<string,unique_ptr<Object>>> objectMaps;
    map<string,map<string,std::any>> anyMaps;
    
    TextureID errorTexture = 0; 

    public:

        void clear() {
            models.clear();
            textures.clear();
            sprites.clear();
            materials.clear();
            actors.clear();
            objectMaps.clear();
            anyMaps.clear();
            //widgets.clear();
            typeInfo.clear();
            particleEffects.clear();
        }

        bool hasModel(string name) {
            return models.contains(name);
        }

        bool hasTexture(string name) {
            return textures.contains(name);
        }

        bool hasSprite(string name) {
            return sprites.contains(name);
        }

        bool hasMaterial(string name) {
            return materials.contains(name);
        }

        bool hasActor(string name) {
            return actors.contains(name);
        }

        bool hasBlock(string name) {
            return has<Block>(name);
        }

        bool hasItem(string name) {
            return has<Item>(name);
        }

        bool hasRecipe(string name) {
            return has<Recipe>(name);
        }

        bool hasParticleEffect(string name) {
            return particleEffects.contains(name);
        }

        bool hasTypeInfo(string name) {
            return typeInfo.contains(name);
        }

        template<typename T>
        bool has(string name) {
            string typePtr = typeid(T).name();
            if constexpr(std::is_convertible_v<T*, Object*>) {
                return objectMaps.contains(typePtr) && objectMaps.at(typePtr).contains(name);
            } else {
                return anyMaps.contains(typePtr) && anyMaps.at(typePtr).contains(name);
            }
            
        }

        Mesh<Vertex>* getModel(string name) {
            if(models.contains(name)) {
                return &models.at(name);
            } else {
                Debug::warn("no model called " + name + "\"");
            }
            return nullptr;
        }
        TextureID getTexture(string name) {
            if(textures.contains(name)) {
                return textures.at(name);
            } else {
                Debug::warn("no texture called \"" + name + "\"");
            }
            return errorTexture;
        }
        Sprite getSprite(string name) {
            if(sprites.contains(name)) {
                return sprites.at(name);
            } else {
                if(textures.contains(name)) {
                    return Sprite(name,textures.at(name));
                }
                Debug::warn("no sprite called \"" + name + "\"");
            }
            return Sprite(errorTexture);
        }
        Material getMaterial(string name) {
            if(materials.contains(name)) {
                return materials.at(name);
            } else {
                Debug::warn("no material called \"" + name + "\"");
            }
            return Material::none;
        }
        Object* getMaterialData(string name) {
            if(materialDataMap.contains(name)) {
                return materialDataMap.at(name).get();
            }
            return nullptr;
        }
        Actor* getActor(string name) {
            if(actors.contains(name)) {
                return actors.at(name).get();
            } else {
                Debug::warn("no actor prototype called \"" + name + "\"");
            }
            return nullptr;
        }

        Recipe* getRecipe(string name) {
            return getPtr<Recipe>(name);
        }

        template<typename T>
        string typedKey(string name) {
            return typeid(T).name() + name;
        }

        template<typename T>
        T* getPtr(string name) {
            return get<T,T*>(name);
        }

        template<typename T,typename ReturnType = T>
        ReturnType get(string name) {
            string typeStr = typeid(T).name();
            string typeName = typeIdToName.contains(typeStr) ? typeIdToName[typeStr] : typeStr;
            if constexpr(std::is_convertible_v<T*, Object*>) {
                if (objectMaps.contains(typeStr) && objectMaps.at(typeStr).contains(name)) {
                    return static_cast<T*>(objectMaps.at(typeStr).at(name).get());
                } else {
                    Debug::warn(typeName + " " + name + " doesn't exist");
                    return nullptr;
                }
            } else {
                if (anyMaps.contains(typeStr) && anyMaps.at(typeStr).contains(name)) {
                    return any_cast<T>(anyMaps.at(typeStr).at(name));
                } else {
                    Debug::warn(typeName + " " + name + " doesn't exist");
                    if(std::is_pointer_v<T>) {
                        return nullptr;
                    } else {
                        return T();
                    }
                }
            }
            
        }

        void addRecipesToVector(std::vector<Recipe*>& recipeList,string category,int maxIngredients) {
            for(auto pair : getRecipes()) {
                Recipe* recipe = pair.second;
                if(recipe->category == category && recipe->ingredients.size() <= maxIngredients) {
                    recipeList.push_back(recipe);
                }
            }
        }

        template<typename T>
        T* getActor(string name) {
            Actor* actor = getActor(name);
            if(actor == nullptr) {
                return nullptr;
            }
            T* typedActor = dynamic_cast<T*>(actor);
            if(typedActor == nullptr) {
                return nullptr;
            }
            return typedActor;
        }

        Block* getBlock(string name) {
            return getPtr<Block>(name);
        }

        Item* getItem(string name) {
            return getPtr<Item>(name);
        }

        string getTypeName(string typeidName) {
            if(typeIdToName.contains(typeidName)) {
                return typeIdToName.at(typeidName);
            }
            return "unknown";
        }

        template<typename T>
        string getTypeName() {
            return getTypeName(typeid(T).name());
        }

        TypeInfo* getTypeInfo(string name) {
            if(typeInfo.contains(name)) {
                return &typeInfo.at(name);
            } else {
                Debug::warn("no type info called \"" + name + "\"");
            }
            return nullptr;
        }

        // template<typename T>
        // T* getWidget(string name) {
        //     Widget* widget = getWidget(name);
        //     if(widget == nullptr) return nullptr;

        //     T* typedWidget = dynamic_cast<T*>(widget);
        //     if(typedWidget != nullptr) {
        //         return typedWidget;
        //     } else {
        //         Debug::warn("widget \"" + name + "\" not of type " + typeid(T).name());
        //         return nullptr;
        //     }
            
        // }

        template<typename T>
        struct ObjectMap {
            std::map<string,unique_ptr<Object>>* map;
            public:
                struct Iterator {

                    std::map<string,unique_ptr<Object>>::iterator iter;
                
                    Iterator(std::map<string,unique_ptr<Object>>::iterator iter) : iter(iter) {

                    }

                    using iterator_category = std::forward_iterator_tag;
                    using difference_type   = std::ptrdiff_t;
                    using value_type        = std::pair<string,T*>;
                    using pointer           = std::pair<string,T*>*;  // or also value_type*
                    using reference         = std::pair<string,T*>&;  // or also value_type&

                    value_type operator*() const { 
                        auto& pair = *iter;
                        return std::pair(pair.first,static_cast<T*>(pair.second.get())); 
                    }
                    // pointer operator->() { 
                    //     std::pair<string,unique_ptr<Object>> pair = *iter;
                    //     return std::pair(pair.first,static_cast<T*>(pair.second.get())); 
                    // }

                    // Prefix increment
                    Iterator& operator++() { iter++; return *this; }  

                    // Postfix increment
                    Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }

                    friend bool operator== (const Iterator& a, const Iterator& b) { return a.iter == b.iter; };
                    friend bool operator!= (const Iterator& a, const Iterator& b) { return a.iter != b.iter; };     
                };

                ObjectMap(Registry* registry) {
                    string typeStr = typeid(T).name();
                    map = &registry->objectMaps[typeStr];
                }
                
                Iterator begin() { return Iterator(map->begin()); }
                Iterator end()   { return Iterator(map->end()); }


                T* operator[](string name) {
                    if(map->contains(name)) {
                        return static_cast<T*>(map->at(name));
                    } else {
                        return nullptr;
                    }
                }
            
        };

        ObjectMap<Item> getItems() {
            return ObjectMap<Item>(this);
        }

        ObjectMap<Block> getBlocks() {
            return ObjectMap<Block>(this);
        }

        template<typename T>
        ObjectMap<T> getObjects() {
            return ObjectMap<T>(this);
        }


        ObjectMap<Recipe> getRecipes() {
            return getObjects<Recipe>();
        }

        map<string,Mesh<Vertex>>& getModels() {
            return models;
        }

        map<string,TextureID>& getTextures() {
            return textures;
        }

        map<string,Material>& getMaterials() {
            return materials;
        }

        ParticleEffect* getParticleEffect(string name) {
            if(particleEffects.contains(name)) {
                return &particleEffects.at(name);
            } else {
                Debug::warn("no effect called \"" + name + "\"");
            }
            return nullptr;
        }

        Mesh<Vertex>* addModel(string name) {
            models.try_emplace(name,Mesh<Vertex>());
            models.at(name).name = name;
            return &models.at(name);
        }

        void setTexture(string name,TextureID texture) {
            textures[name] = texture;
            if(name == "error") {
                errorTexture = texture;
            }
        }
        void addSprite(string name,Sprite sprite) {
            sprite.name = name;
            sprites.emplace(name,sprite);
        }
        void addRecipe(string name,Recipe recipe) {
            addObject<Recipe>(name,std::make_unique<Recipe>(recipe));
        }


        void addMaterial(string name,Material material) {
            material.name = name;
            materials.emplace(name,material);
        }
        template <typename T>
        void addMaterial(string name,Material material,T materialData) {
            addMaterial(name,material);
            //materialDataMap[name] = std::make_unique<T>(materialData);
        }

        template <typename T>
        T* addActor(string name) {
            auto actor = T::makeDefaultPrototype();
            actor->name = name;
            actors.emplace(name,std::move(actor));
            return dynamic_cast<T*>(actors.at(name).get());
        }

        template <typename T>
        T* addBlock(string name) {
            // auto block = std::make_unique<T>();
            // block->name = name;
            // blocks.emplace(name,std::move(block));
            // return dynamic_cast<T*>(blocks.at(name).get());
            return addObject<Block,T>(name);
        }

        template <typename T>
        T* addItem(string name) {
            return addObject<Item,T>(name);
        }

        void addParticleEffect(string name,ParticleEffect effect) {
            particleEffects.emplace(name,effect);
        }

        template<typename BaseType,typename ObjType = BaseType>
        ObjType* addObject(string name) {
            string typeStr = typeid(BaseType).name();
            objectMaps[typeStr].emplace(name,std::make_unique<ObjType>());
            objectMaps[typeStr].at(name)->name = name;
            return static_cast<ObjType*>(objectMaps[typeStr].at(name).get());
        }

        template<typename BaseType>
        BaseType* addObject(string name,std::unique_ptr<BaseType> objPtr) {
            if(objPtr == nullptr) {
                Debug::warn("object added to registry is null");
                return nullptr;
            }
            string typeStr = typeid(BaseType).name();
            objectMaps[typeStr].emplace(name,std::move(objPtr));
            objectMaps[typeStr].at(name)->name = name;
            return static_cast<BaseType*>(objectMaps[typeStr].at(name).get());
        }

        template<typename T>
        void addAny(string name,T object) {
            string typeStr = typeid(T).name();
            anyMaps[typeStr][name] = std::any(object);
        }

        template<typename T>
        TypeInfo* addTypeInfo(string name) {
            typeInfo.emplace(std::piecewise_construct,std::make_tuple(name),std::make_tuple(name));
            typeIdToName[typeid(T).name()] = name;
            return &typeInfo.at(name);
        }

        VkPipeline litShader;
        VkPipeline textShader;
        VkPipeline uiShader;

        Font font;

};