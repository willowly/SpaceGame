#include "api/type-info.hpp"
#include "imgui/imgui.h"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "imgui/misc/fonts/IconsLucide.h"
#include "api/asset-serializer.hpp"
#include "engine/debug.hpp"
#include "graphics/material/material-object.hpp"
#include <array>
#include <cstring>
#include <memory>
#include <type_traits>

namespace DebugMenu {


    class AssetViewer {

        ImFont* iconFont = nullptr;

        struct TextureObject : public Object {
            TextureID id = 0;
            ImTextureID imguiTextureID = 0;
            TextureInfo info;
            bool hasTexture = false;

            void load(TextureID id,Vulkan* vulkan) {
                this->id = id;
                if(!hasTexture && vulkan != nullptr) {
                    imguiTextureID = vulkan->getImGuiTextureID(id);
                    info = vulkan->getTextureInfo(id);
                    hasTexture = true;
                }
            }

            string getTypeName() override {return "texture_obj";}
        };

        struct AnyObject : public Object {
            std::any value;
            TypeInfo* typeInfo;

            string getTypeName() override {return "any_obj";}
        };

        struct MetaData {
            bool unsaved = false;
        };

        using DisplayFunction = bool(AssetViewer*,const char*,std::any&,bool&);

        std::map<string,TextureObject> textureObjects;
        std::map<string,AnyObject> anyObjects;
        std::map<string,std::function<DisplayFunction>> displayFunctions;
        AssetSerializer serializer;

        std::array<char, 1000> searchBuffer;
        std::array<char, 1000> nameBuffer;

        using CachedObjectMap = std::map<string,Object*>;
        std::map<string,CachedObjectMap> cachedObjectMaps;
        using CachedAnyMap = std::map<string,std::any>;
        std::map<string,CachedAnyMap> cachedAnyMaps;

        using MetaDataMap = std::map<string,MetaData>;
        std::map<string,MetaDataMap> metaDataMaps;

        bool objectMapsOutOfDate = true;

        string createObjectPopup;

        template<typename T>
        void loadCachedObjectMap(string name,T iter) {
            cachedObjectMaps[name].clear();
            for (auto pair : iter) {
                cachedObjectMaps[name][pair.first] = pair.second;
            }
        }

        template<typename T>
        void loadCachedAnyMap(string name,T iter) {
            cachedAnyMaps[name].clear();
            for (auto pair : iter) {
                cachedAnyMaps[name][pair.first] = pair.second;
            }
        }

        void refreshCachedObjectMaps() {
            assert(registry != nullptr);
            loadCachedObjectMap("block",registry->getBlocks());
            loadCachedObjectMap("item",registry->getItems());
            loadCachedObjectMap("material_object",registry->getObjects<MaterialObject>());
            loadCachedObjectMap("widget",registry->getObjects<Widget>());

            loadCachedAnyMap("font",registry->getAnys<Font>());
            // loadCachedObjectMap("item_slot_widget",registry->getObjects<ItemSlotWidget>());
            // loadCachedObjectMap("furnace_widget",registry->getObjects<FurnaceWidget>());
            // loadCachedObjectMap("toolbar_widget",registry->getObjects<ToolbarWidget>());
            // loadCachedObjectMap("inventory_widget",registry->getObjects<InventoryWidget>());
            // loadCachedObjectMap("player_widget",registry->getObjects<PlayerWidget>());
            //loadCachedObjectMap("actor",registry->getActors());
            
        }

        void cacheWidgetObjectMap(string typeName) {
            TypeInfo* typeInfo = registry->getTypeInfo(typeName);
            if(typeInfo == nullptr) {
                Debug::warn("no type info for " + typeName);
                return;
            }
            cachedObjectMaps[typeName].clear();
            for (auto pair : registry->getObjects<Widget>())
            {
                cachedObjectMaps[typeName][pair.first] = pair.second;
            }
        }

        MetaData& getMetaData(Object* obj) {
            assert(obj != nullptr);
            auto& map = metaDataMaps[obj->getTypeName()];
            return map[obj->name];
        }
        
        template<typename T,typename Iterable>
        void tab(string name,Iterable map) {
            if(ImGui::BeginTabItem(name.c_str())) {
                for (auto pair : map)
                {
                    string name = pair.first;
                    Object* obj = nullptr;
                    bool unsaved = false;
                    if constexpr(std::is_same_v<T,Material>) {
                        obj = registry->getMaterialData(name);
                    } else if constexpr(std::is_same_v<T,TextureID>) {
                        textureObjects[name].load(pair.second,vulkan);
                        obj = &textureObjects[name];
                    } else if constexpr(std::is_same_v<T,Recipe>) {
                        obj = registry->getRecipe(name);
                    } else if constexpr(std::is_same_v<T,Mesh<Vertex>>) {
                        obj = registry->getModel(name);
                    } else {
                        obj = pair.second;
                        if(getMetaData(obj).unsaved) {
                            unsaved = true;
                        }
                    }
                    ImGui::PushID(name.c_str());
                    bool isSelected = selectedObject == obj;
                    if(unsaved) {
                        name += " (unsaved)";
                        ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1,1,0,1));
                    }
                    if(ImGui::Selectable(name.c_str(),selectedObject)) {
                        selectedObject = obj;
                    }
                    if(unsaved) {
                        ImGui::PopStyleColor();
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }
        }

        void widgetSection(TypeInfo* info) {
            string label = prettyPrintLabel(info->getName());
            if(ImGui::CollapsingHeader(label.c_str())) {
                for (auto pair : registry->getObjects(info->getTypeId()))
                {
                    string name = pair.first;
                    Object* obj = pair.second;
                    ImGui::PushID(name.c_str());
                    bool isSelected = selectedObject == obj;
                    if(ImGui::Selectable(name.c_str(),isSelected)) {
                        selectedObject = obj;
                    }
                    ImGui::PopID();
                }
            }
        }

        template<typename T>
        void anySection(string name) {
            string typeInfoName = registry->getTypeName(typeid(T).name());
            TypeInfo* typeInfo = registry->getTypeInfo(typeInfoName);
            if(typeInfo == nullptr) {
                return;
            }
            if(ImGui::CollapsingHeader(name.c_str())) {
                for (auto pair : registry->getAnys<T>())
                {
                    string name = pair.first;
                    auto* value = pair.second;
                    ImGui::PushID(name.c_str());
                    if(ImGui::Selectable(name.c_str(),false)) {
                        anyObjects[name].value = value;
                        anyObjects[name].typeInfo = typeInfo;
                        selectedObject = &anyObjects[name];
                    }
                    ImGui::PopID();
                }
            }
            
        }
        
        void textureInspector(TextureObject* obj) {
            ImGui::Text("Texture ID %i",obj->id);
            if(obj->hasTexture) {
                displayImage(imageDisplaySizeLarge,obj->imguiTextureID,obj->info.width,obj->info.height);
                ImGui::Text("%ix%i",obj->info.width,obj->info.height);
            } else {
                ImGui::Text("Texture could not be loaded (vulkan is null?)");
            }
        }

        void displayImage(float displaySize,ImTextureID  id,int textureWidth,int textureHeight,Rect rect = Rect::unitSquare) {
            float width = displaySize;
            float height = ((float)textureHeight/textureWidth) * displaySize;
            ImVec2 uv0;
            uv0.x = rect.position.x;
            uv0.y = rect.position.y + rect.size.y;
            ImVec2 uv1;
            uv1.x = rect.position.x + rect.size.x;
            uv1.y = rect.position.y;
            ImGui::Image(id,ImVec2(width,height),uv0,uv1);
        }

        
        void saveAsset(Object* obj,TypeInfo* info) {
            string path = "..\\..\\assets";
            if(std::filesystem::exists(path)) {
                string folder = info->getFolderName();
                std::ofstream file(path + "\\" + folder + "\\" + obj->name + ".asset"); 
                serializer.serialize(obj,info,file,0,true);
                getMetaData(obj).unsaved = false;
                file.close();
            } else {
                Debug::warn("couldn't find assets path");
            }
        }
        
        void objectInspector(Object* obj) {
            
            if(obj->getTypeName() == "texture_obj") {
                TextureObject* textureObj = dynamic_cast<TextureObject*>(obj);
                textureInspector(textureObj);
                return;
            }

            

            if(obj->getTypeName() == "any_obj") {
                AnyObject* anyObj = dynamic_cast<AnyObject*>(obj);
                anyInspector(anyObj);
                return;
            }
            auto typeInfo = registry->getTypeInfo(obj->getTypeName());
            if(typeInfo == nullptr) {
                ImGui::Text("Type Info is null (%s)",obj->getTypeName().c_str());
            } else {
                if(ImGui::Button("Save")) {
                    saveAsset(obj,typeInfo);
                }
                bool changed = false;
                displayProperties(obj,typeInfo,changed);
                if(changed) {
                    getMetaData(obj).unsaved = true;
                    std::cout << "CHANGED!" << std::endl;
                }
            }
        }

        void anyInspector(AnyObject* obj) {
            // if(ImGui::Button("Save")) {
            //     saveAsset(obj,obj->typeInfo);
            // }
            bool changed = false;
            displayProperties(obj->value,obj->typeInfo,changed);
            if(changed) {
                getMetaData(obj).unsaved = true;
                std::cout << "CHANGED!" << std::endl;
            }
        }
        
        void inspector(Object* obj) {
            ImGui::Begin("Inspector");
            objectInspector(obj);
            ImGui::End();
        }

        template<typename T>
        void displayProperties(T obj,TypeInfo* info,bool& changed) {
            string typeName = prettyPrintLabel(info->getName());
            ImGui::SeparatorText(typeName.c_str());
            for(auto& property : info->getProperties()) {
                displayProperty(obj,property.get(),changed);
            }
            if(info->getParent() != nullptr) {
                displayProperties(obj,info->getParent(),changed);
            }
            
        }
        
        static string prettyPrintLabel(string str) {
            if(str.length() == 0) return str;
            
            
            size_t start = 0;
            while(true) {
                str[start] = toupper(str[start]);
                start = str.find("_",start);
                if(start == string::npos) break;
                str[start] = ' ';
                start++;
                if(start >= str.length()) break;
            }
            return str;
        }

        template<typename T>
        void displayProperty(T obj,GenericPropertyInfo* property,bool& changed) {
            string stringLabelPretty = prettyPrintLabel(property->name);
            const char* label = stringLabelPretty.c_str();
            string typeIdName = property->typeIdName();
            if(displayFunctions.contains(property->typeIdName())) {
                auto value = property->get(obj);
                auto func = displayFunctions.at(typeIdName);
                if(func(this,label,value,changed)) {
                    property->set(obj,value);
                }
                return;
            }
            switch(property->getPropertyType()) {
                case PropertyTypeEnum::Float:
                    displayFloat(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Int:
                    displayInt(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Enum:
                    displayEnum(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Bool:
                    displayBool(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::String:
                    displayString(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Texture:
                    displayTexture(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Vector:
                    displayVector(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Material:
                    displayMaterial(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::ObjectPointer:
                    displayObjectReference(label,obj,property,changed);
                    break;
                case PropertyTypeEnum::Pointer:
                    displayPointer(label,obj,property,changed);
                    break;
                default:
                    displayComposite(label,obj,property,changed);
                break;
            }
        }

        enum class SelectorResponse {
            None,
            Goto,
            Change
        };

        SelectorResponse selector(const char* label,string name) {
            ImGui::PushID(label);
            if(ImGui::Button(name.c_str(),ImVec2(300,ImGui::GetFrameHeight()))) {
                ImGui::PopID();
                return SelectorResponse::Goto;
            }
            ImGui::SameLine();
            ImGui::PushFont(iconFont);
            if(ImGui::Button(ICON_LC_CIRCLE_DOT,ImVec2(ImGui::GetFrameHeight(),ImGui::GetFrameHeight()))) {
                ImGui::PopID();
                ImGui::PopFont();
                return SelectorResponse::Change;
            }
            ImGui::PopFont();
            ImGui::SameLine();
            ImGui::Text(label);
            ImGui::PopID();
            return SelectorResponse::None;
        }
        
        template<typename T>
        void displayBool(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            bool f = property->get<bool>(obj);
            if(ImGui::Checkbox(label,&f)) {
                property->set<bool>(obj,f);
                changed = true;
            }
        }
        
        template<typename T>
        void displayInt(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            int f = property->get<int>(obj);
            if(ImGui::InputInt(label,&f)) {
                property->set<int>(obj,f);
                changed = true;
            }
        }

        template<typename T>
        void displayEnum(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            int f = property->getEnumValue(obj);
            if(ImGui::InputInt(label,&f)) {
                property->setEnumValue(obj,f);
                changed = true;
            }
        }
        
        template<typename T>
        void displayFloat(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            float f = property->get<float>(obj);
            if(ImGui::InputFloat(label,&f)) {
                property->set<float>(obj,f);
                changed = true;
            }
        }


        bool displayRect(const char* label,std::any& value,bool& changed) {
            Rect r = std::any_cast<Rect>(value);
            if(displayRectRef(label,r,changed)) {
                value = r;
                return true;
            }
            return false;
        }

        bool displayRectRef(const char* label,Rect& r,bool& changed) {
            float list[] = {r.position.x,r.position.y,r.size.x,r.size.y};
            if(ImGui::InputFloat4(label,list)) {
                r = Rect(list[0],list[1],list[2],list[3]);
                changed = true;
                return true;
            }
            return false;
        }
        
        template<typename T>
        void displayString(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            string str = property->get<string>(obj);
            char buffer[1000];
            std::strcpy(buffer,str.c_str());
            if(ImGui::InputText(label,buffer,1000)) {
                string s = buffer;
                property->set<string>(obj,s);
                changed = true;
            }
        }
        
        template<typename T>
        void displayObjectReference(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            Object* ref = property->getObj(obj);

            string typeName = "null";
            string refName = "null";
            if(ref != nullptr) {
                typeName = ref->getTypeName();
                refName = ref->name;
            }
            string propertyTypeName = registry->getTypeName(property->typeIdNameNonPointer());
            auto propertyType = registry->getTypeInfo(propertyTypeName);
            if(propertyType == nullptr) {
                Debug::warn("no type info for " + propertyTypeName);
                return;
                propertyTypeName = propertyType->getRootName();
            }

            ImGui::PushID(label);
            auto response = selector(label,(refName + " (" + typeName + ")"));
            if(response == SelectorResponse::Goto) {
                selectedObject = ref;
            }
            if(response == SelectorResponse::Change) {
                openSelectorPopup("ObjectRefPopup");
            }
            if(cachedObjectMaps.contains(propertyTypeName)) {
                if(selectorPopup("ObjectRefPopup",cachedObjectMaps[propertyTypeName],refName,ref)) {
                    property->set(obj,ref);
                    changed = true;
                }
            } else {
                cacheWidgetObjectMap(propertyTypeName);
            }
            ImGui::PopID();
        }

        template<typename T>
        void displayPointer(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            void* ptr = property->getVoidPtr(obj);
            auto ref = property->get(obj);
            string typeName = "null";
            string refName = "null";
            if(ptr != nullptr) {
                typeName = registry->getTypeName(property->typeIdNameNonPointer());
                refName = registry->getPointerName(property->typeIdNameNonPointer(),ptr);
            }
            ImGui::PushID(label);
            auto response = selector(label,(refName + " (" + typeName + ")"));
            if(response == SelectorResponse::Change) {
                openSelectorPopup("PointerPopup");
            }
            if(selectorPopup("PointerPopup",cachedAnyMaps[typeName],refName,ref)) {
                property->set(obj,ref);
                changed = true;
            }
            ImGui::PopID();
        }

        bool displaySprite(const char* label,std::any& value,bool& changed) {
            Sprite sprite = std::any_cast<Sprite>(value);
            
            if(vulkan != nullptr) {
                auto info = vulkan->getTextureInfo(sprite.texture);
                displayImage(imageDisplaySizeSmall,vulkan->getImGuiTextureID(sprite.texture),info.width,info.height,sprite.rect);
            }
            bool thisChanged = false;
            if(ImGui::TreeNode(label)) {
                displayTexture("texture",sprite.texture,thisChanged);
                displayRectRef("rect",sprite.rect,thisChanged);
                ImGui::TreePop();
            }
            if(thisChanged) {
                value = sprite;
                changed = true;
                return true;
            }
            return false;
            
            //ImGui::LabelText(label,"%s",sprite.name.c_str());
        }

        bool displayVec3(const char* label,std::any& value,bool& changed) {
            vec3 f = std::any_cast<vec3>(value);
            float list[] = {f.x,f.y,f.z};
            if(ImGui::InputFloat3(label,list)) {
                value = vec3(list[0],list[1],list[2]);
                changed = true;
                return true;
            }
            return false;
        }

        bool displayVec2(const char* label,std::any& value,bool& changed) {
            vec2 f = std::any_cast<vec2>(value);
            float list[] = {f.x,f.y};
            if(ImGui::InputFloat2(label,list)) {
                value = vec2(list[0],list[1]);
                changed = true;
                return true;
            }
            return false;
        }

        bool displayQuat(const char* label,std::any& value,bool& changed) {
            quat f = std::any_cast<quat>(value);
            vec3 v = glm::degrees(glm::eulerAngles(f));
            float list[] = {v.x,v.y,v.z};
            if(ImGui::InputFloat3(label,list)) {
                value = glm::quat(glm::radians(vec3(list[0],list[1],list[2])));
                changed = true;
                return true;
            }
            return false;
        }

        bool displayColor(const char* label,std::any& value,bool& changed) {
            Color f = std::any_cast<Color>(value);
            float list[] = {f.r,f.g,f.b,f.a};
            if(ImGui::ColorEdit4(label,list)) {
                value = Color(list[0],list[1],list[2],list[3]);
                changed = true;
                return true;
            }
            return false;
        }


        void openSelectorPopup(string id) {
            ImGui::OpenPopup(id.c_str());
            strcpy(searchBuffer.data(), "");

            if(objectMapsOutOfDate) {
                refreshCachedObjectMaps();
            }
            ImGui::SetKeyboardFocusHere(1);
        }

        template<bool texturePopup = false,typename PropertyType,typename PropertyMapType>
        bool selectorPopup(string id,map<string, PropertyMapType> & map,string selectedName,PropertyType& propertyValue) {
            if(ImGui::BeginPopup(id.c_str())) {
                ImGui::InputText("##Search", searchBuffer.data(), searchBuffer.size());
                if(ImGui::BeginListBox("##List")) {
                    for(auto& pair : map) {

                        if(pair.first.find(searchBuffer.data(),0) == string::npos) continue;
                        
                        if(ImGui::Selectable(pair.first.c_str(),selectedName == pair.first)) {
                            if constexpr(std::is_same_v<PropertyMapType,PropertyType>) {
                                propertyValue = pair.second;
                            }
                            if constexpr(std::is_same_v<PropertyMapType*,PropertyType>) {
                                propertyValue = &pair.second;
                            }
                            ImGui::CloseCurrentPopup();
                            ImGui::EndListBox();
                            ImGui::End();
                            return true;
                        }
                        if constexpr(texturePopup) {
                            if(ImGui::IsItemHovered()) {
                                if(ImGui::BeginTooltip()) {
                                    auto info = vulkan->getTextureInfo(pair.second);
                                    displayImage(100,vulkan->getImGuiTextureID(pair.second), info.width, info.height);
                                    ImGui::EndTooltip();
                                }
                            }
                        }
                    }
                    ImGui::EndListBox();
                }
                ImGui::End();
            }
            return false;
        }

        template<typename T>
        void displayMaterial(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            Material material = property->get<Material>(obj);

            ImGui::PushID(label);
            auto response = selector(label,material.name);
            if(response == SelectorResponse::Change) {
                openSelectorPopup("MaterialPopup");
            }
            if(selectorPopup("MaterialPopup",registry->getMaterials(),material.name,material)) {
                property->set(obj,material);
                changed = true;
            }
            ImGui::PopID();
            if(response == SelectorResponse::Goto) {
                auto data = registry->getMaterialData(material.name);
                if(data != nullptr) {
                    selectedObject = data;
                }
            }
        }

        
        bool displayTexture(const char* label,TextureID& id,bool& changed,bool showImage = true) {
            
            if(vulkan != nullptr) {
                auto info = vulkan->getTextureInfo(id);
                if(showImage) displayImage(imageDisplaySizeSmall,vulkan->getImGuiTextureID(id),info.width,info.height);
                auto response = selector(label,info.name);
                if(response == SelectorResponse::Goto) {
                    textureObjects[info.name].load(id,vulkan);
                    textureObjects[info.name].name = info.name;
                    selectedObject = &textureObjects[info.name];
                }
                if(response == SelectorResponse::Change) {
                    openSelectorPopup("TexturePopup");
                }
                if(selectorPopup<true>("TexturePopup",registry->getTextures(),info.name,id)) {
                    changed = true;
                    return true;
                }
            } else {
                ImGui::LabelText(label,"%i",id);
            }
            return false;
        }

        bool displayMesh(const char* label,std::any& value,bool& changed) {


            ImGui::PushID(label);
            auto mesh = std::any_cast<Mesh<Vertex>*>(value);

            string meshName = "null";
            if(mesh != nullptr) {
                meshName = mesh->name;
            }
            
            auto response = selector(label,meshName);
            if(response == SelectorResponse::Goto) {
                selectedObject = mesh;
            }
            if(response == SelectorResponse::Change) {
                openSelectorPopup("MeshPopup");
            }
            if(selectorPopup("MeshPopup",registry->getModels(),meshName,mesh)) {
                value = mesh;
                changed = true;
                ImGui::PopID();
                return true;
            }
            ImGui::PopID();
            return false;
        }

        template<typename T>
        void displayTexture(const char* label,T obj,GenericPropertyInfo* property,bool showImage = true) {
            TextureID texture = property->get<TextureID>(obj);

            if(displayTexture(label,texture,showImage)) {
                property->set<TextureID>(obj,texture);
            }
        }

        template<typename T>
        void displayComposite(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            string typeName = registry->getTypeName(property->typeIdName());
            TypeInfo* info = registry->getTypeInfo(typeName);
            if(info == nullptr) {
                ImGui::LabelText(property->name.c_str(),"???");
                return;
            }
            // default display function
            if(ImGui::TreeNode(label)) {
                displayProperties(property->getPtr(obj),info,changed);
                ImGui::TreePop();
            }

            
        }

        template<typename T>
        void displayVector(const char* label,T obj,GenericPropertyInfo* property,bool& changed) {
            if(ImGui::TreeNode(label)) {
                for (size_t i = 0; i < property->getSize(obj); i++)
                {
                    displayProperty(property->getPtr(obj),property->getElement(i),changed);
                }
                if(ImGui::Button("Add")) {
                    property->setSize(obj,property->getSize(obj)+1);
                }
                ImGui::SameLine();
                if(ImGui::Button("Remove")) {
                    property->setSize(obj,property->getSize(obj)-1);
                }
                ImGui::TreePop();
            }

            
        }

        template<typename T>
        void newObject(string name,TypeInfo* info) {
            if(!info->constructorFunction) {
                Debug::warn("Couldn't deserialize file, no object constructor");
                return;
            }

            auto uniquePtrObj = info->constructorFunction();

            auto rawTypedPtr = dynamic_cast<T*>(uniquePtrObj.get());
            uniquePtrObj.release();

            auto uniqueTypedPtr = std::unique_ptr<T>(rawTypedPtr);

            if(uniqueTypedPtr == nullptr) {
                return;
            }

            
            auto ptr = registry->addObject<T>(name,std::move(uniqueTypedPtr));
            getMetaData(ptr).unsaved = true;
        }

        void newObjectMenu(string name) {
            string label = "New " + prettyPrintLabel(name);
            if(ImGui::BeginMenu(label.c_str())) {
                for (auto info : registry->getTypeInfo(name)->derived)
                {
                    if(!info->constructorFunction) {
                        continue;
                    }
                    auto label = "New " + prettyPrintLabel(info->getName());
                    strcpy(nameBuffer.data(), "");
                    if(ImGui::Selectable(label.c_str())) {
                        createObjectPopup = label;
                    }
                }
                
                ImGui::EndMenu();
            }
        }

        template<typename T>
        void newObjectModal(string name) {
            for (auto info : registry->getTypeInfo(name)->derived)
            {
                if(!info->constructorFunction) {
                    continue;
                }
                auto label = "New " + prettyPrintLabel(info->getName());
                if(createObjectPopup == label) {
                    ImGui::OpenPopup(label.c_str());
                }
                if(ImGui::BeginPopupModal(label.c_str(),NULL,ImGuiWindowFlags_AlwaysAutoResize)) {
                    if(createObjectPopup != "") {
                        createObjectPopup = "";
                        ImGui::SetKeyboardFocusHere();
                    }
                    ImGui::InputText("Name",nameBuffer.data(),nameBuffer.size());
                    if(ImGui::Button("Create")) {
                        newObject<T>((string)nameBuffer.data(),info);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Cancel")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                    
                }
            }
        }

        void addDefaultDisplayFunctions() {
            addDisplayFunction<Sprite>(&AssetViewer::displaySprite);
            addDisplayFunction<Rect>(&AssetViewer::displayRect);
            addDisplayFunction<Mesh<Vertex>*>(&AssetViewer::displayMesh);
            addDisplayFunction<vec3>(&AssetViewer::displayVec3);
            addDisplayFunction<quat>(&AssetViewer::displayQuat);
            addDisplayFunction<Color>(&AssetViewer::displayColor);
            addDisplayFunction<vec2>(&AssetViewer::displayVec2);
        }

    
        public:

            AssetViewer() {
                addDefaultDisplayFunctions();
                strcpy(searchBuffer.data(), "");
            }
            Registry* registry = nullptr;
            Vulkan* vulkan = nullptr;
            Object* selectedObject = nullptr;
            float imageDisplaySizeLarge = 300;
            float imageDisplaySizeSmall = 100;

            template<typename T>
            void addDisplayFunction(std::function<DisplayFunction> function) {
                displayFunctions[typeid(T).name()] = function;
            }

            template<typename T>
            void removeDisplayFunction() {
                displayFunctions.erase(typeid(T).name());
            }



            void reload() {
                selectedObject = nullptr;
            }
            
            void display() {
                assert(registry != nullptr);

                serializer.registry = registry;
                serializer.vulkan = vulkan;

                if(iconFont == nullptr) {
                    ImGuiIO& io = ImGui::GetIO();
                    #ifdef __APPLE__
                        iconFont = io.Fonts->AddFontFromFileTTF("assets/fonts/lucide.ttf");
                    #endif
                    #ifndef __APPLE__
                        iconFont = io.Fonts->AddFontFromFileTTF("assets\\fonts\\lucide.ttf");
                    #endif
                }
                bool open = true;
                ImGui::Begin("Registry",&open,ImGuiWindowFlags_MenuBar);

                ImGui::BeginMenuBar();
                if(ImGui::MenuItem("New",NULL,true)) {
                    ImGui::OpenPopup("New");
                }
                if(ImGui::BeginPopup("New")) {
                    newObjectMenu("item");
                    newObjectMenu("block");
                    newObjectMenu("material_object");
                    newObjectMenu("actor");
                    newObjectMenu("widget");
                    ImGui::EndPopup();
                }

                newObjectModal<Item>("item");
                newObjectModal<Block>("block");
                newObjectModal<MaterialObject>("material_object");
                newObjectModal<Actor>("actor");
                newObjectModal<Widget>("widget");

    

                // if(ImGui::MenuItem("Save All",NULL,true)) {
                //     ImGui::OpenPopup("Save All?"); 
                // }
                // if(ImGui::BeginPopupModal("Save All?")) {
                //     ImGui::Text("Are you sure?");
                //     if(ImGui::Button("Save All")) {
                //         std::cout << "saved all" << std::endl;
                //         ImGui::CloseCurrentPopup();
                //     }
                //     ImGui::SameLine();
                //     if(ImGui::Button("Cancel")) {
                //         ImGui::CloseCurrentPopup();
                //     }
                //     ImGui::EndPopup();
                // }
                ImGui::EndMenuBar();
                
                ImGui::BeginTabBar("RegistryBar");
                
                tab<Actor>("Actors",registry->getActors());
                tab<Item>("Items",registry->getItems());
                tab<Block>("Blocks",registry->getBlocks());
                tab<Recipe>("Recipes",registry->getRecipes());
                tab<MaterialObject>("Materials",registry->getObjects<MaterialObject>());
                tab<TextureID>("Textures",registry->getTextures());
                tab<Mesh<Vertex>>("Models",registry->getModels());
                tab<Widget>("Widgets",registry->getObjects<Widget>());
                // if(ImGui::BeginTabItem("Widgets")) {

                //     auto widgetTypeInfo = registry->getTypeInfo<Widget>();
                //     for (auto widgetTypeInfo :  widgetTypeInfo->derived)
                //     {
                //         widgetSection(widgetTypeInfo);
                //     }
                //     ImGui::EndTabItem();
                // }

                if(ImGui::BeginTabItem("Terrain")) {

                    anySection<TerrainType>("Terrain Types");
                    anySection<TerrainSettings>("Terrain Settings");
                    ImGui::EndTabItem();
                }
                
                ImGui::EndTabBar();
                
                ImGui::End();
                
                    if(selectedObject != nullptr) {
                        inspector(selectedObject);
                    }
                    

                }

            

    };

}