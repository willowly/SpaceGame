#include "imgui/imgui.h"
#include "engine/registry.hpp"
#include "engine/loader.hpp"
#include "imgui/misc/fonts/IconsLucide.h"

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

            string getTypeName() override {return "texture_obj";};
        };

        std::map<string,TextureObject> textureObjects;
        
        template<typename T>
        void tab(string name,std::map<string,T>& map) {
            if(ImGui::BeginTabItem(name.c_str())) {
                for (auto& pair : map)
                {
                    string name = pair.first;
                    Object* obj = nullptr;
                    if constexpr(std::is_same_v<T,Material>) {
                        obj = registry->getMaterialData(name);
                    } else if constexpr(std::is_same_v<T,TextureID>) {
                        textureObjects[name].load(pair.second,vulkan);
                        obj = &textureObjects[name];
                    } else if constexpr(std::is_same_v<T,Recipe>) {
                        obj = &pair.second;
                    } else if constexpr(std::is_same_v<T,Mesh<Vertex>>) {
                        obj = &pair.second;
                    }else{
                        obj = pair.second.get();
                    }
                    ImGui::PushID(name.c_str());
                    bool isSelected = selectedObject == obj;
                    if(ImGui::Selectable(name.c_str(),selectedObject)) {
                        selectedObject = obj;
                    }
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
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
        
        void objectInspector(Object* obj) {
            
            if(obj->getTypeName() == "texture_obj") {
                TextureObject* textureObj = dynamic_cast<TextureObject*>(obj);
                textureInspector(textureObj);
                return;
            }
            
            auto typeInfo = registry->getTypeInfo(obj->getTypeName());
            if(typeInfo == nullptr) {
                ImGui::Text("Type Info is null (%s)",obj->getTypeName().c_str());
            } else {
                displayProperties(obj,typeInfo);
            }
        }
        
        void inspector(Object* obj) {
            ImGui::Begin("Inspector");
            objectInspector(obj);
            ImGui::End();
        }
        
        void displayProperties(Object* obj,TypeInfo* info) {
            string typeName = prettyPrintLabel(info->getName());
            ImGui::SeparatorText(typeName.c_str());
            for(auto& property : info->getProperties()) {
                displayProperty(obj,property.get());
            }
            if(info->parent != nullptr) {
                displayProperties(obj,info->parent);
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
        
        void displayProperty(Object* obj,GenericPropertyInfo* property) {
            string stringLabelPretty = prettyPrintLabel(property->name);
            const char* label = stringLabelPretty.c_str();
            switch(property->getPropertyType()) {
                case PropertyTypeEnum::Float:
                    displayFloat(label,obj,property);
                    break;
                case PropertyTypeEnum::Int:
                    displayInt(label,obj,property);
                    break;
                case PropertyTypeEnum::Bool:
                    displayBool(label,obj,property);
                    break;
                case PropertyTypeEnum::String:
                    displayString(label,obj,property);
                    break;
                case PropertyTypeEnum::Vec3:
                    displayVec3(label,obj,property);
                    break;
                case PropertyTypeEnum::Vec4:
                    displayVec4(label,obj,property);
                    break;
                case PropertyTypeEnum::Quat:
                    displayQuat(label,obj,property);
                    break;
                case PropertyTypeEnum::Sprite:
                    displaySprite(label,obj,property);
                    break;
                case PropertyTypeEnum::Texture:
                    displayTexture(label,obj,property);
                    break;
                case PropertyTypeEnum::Material:
                    displayMaterial(label,obj,property);
                    break;
                case PropertyTypeEnum::ObjectPointer:
                    displayObjectReference(label,obj,property);
                    break;
                default:
                ImGui::LabelText(property->name.c_str(),"???");
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
            if(ImGui::Button("w",ImVec2(ImGui::GetFrameHeight(),ImGui::GetFrameHeight()))) {
                ImGui::PopID();
                return SelectorResponse::Change;
            }
            ImGui::SameLine();
            ImGui::Text(label);
            ImGui::PopID();
            return SelectorResponse::None;
        }
        
        void displayBool(const char* label,Object* obj,GenericPropertyInfo* property) {
            bool f = property->get<bool>(obj);
            ImGui::Checkbox(label,&f);
            property->set<bool>(obj,f);
        }
        
        void displayInt(const char* label,Object* obj,GenericPropertyInfo* property) {
            int f = property->get<int>(obj);
            ImGui::InputInt(label,&f);
            property->set<int>(obj,f);
        }
        
        void displayFloat(const char* label,Object* obj,GenericPropertyInfo* property) {
            float f = property->get<float>(obj);
            ImGui::InputFloat(label,&f);
            property->set<float>(obj,f);
        }
        
        void displayVec3(const char* label,Object* obj,GenericPropertyInfo* property) {
            vec3 f = property->get<vec3>(obj);
            float list[] = {f.x,f.y,f.z};
            ImGui::InputFloat3(label,list);
            property->set<vec3>(obj,vec3(list[0],list[1],list[2]));
        }

        void displayVec4(const char* label,Object* obj,GenericPropertyInfo* property) {
            vec4 f = property->get<vec4>(obj);
            float list[] = {f.x,f.y,f.z,f.w};
            if(ImGui::InputFloat4(label,list)) {
                property->set<vec3>(obj,vec4(list[0],list[1],list[2],list[3]));
            }
        }
        
        void displayQuat(const char* label,Object* obj,GenericPropertyInfo* property) {
            quat f = property->get<quat>(obj);
            vec3 v = glm::degrees(glm::eulerAngles(f));
            float list[] = {v.x,v.y,v.z};
            if(ImGui::InputFloat3(label,list)) {
                quat q = glm::quat(glm::radians(vec3(vec3(list[0],list[1],list[2]))));
                property->set<quat>(obj,q);
            }
        }
        
        void displayIVec3(const char* label,Object* obj,GenericPropertyInfo* property) {
            ivec3 f = property->get<ivec3>(obj);
            int list[] = {f.x,f.y,f.z};
            ImGui::InputInt3(label,list);
            property->set<ivec3>(obj,ivec3(list[0],list[1],list[2]));
        }

        bool displayRect(const char* label,Rect& r) {
            float list[] = {r.position.x,r.position.y,r.size.x,r.size.y};
            if(ImGui::InputFloat4(label,list)) {
                r = Rect(list[0],list[1],list[2],list[3]);
                return true;
            }
            return false;
        }

        void displayRect(const char* label,Object* obj,GenericPropertyInfo* property) {
            Rect r = property->get<Rect>(obj);
            if(displayRect(label,r)) {
                property->set<Rect>(obj,r);
            }
        }
        
        void displayString(const char* label,Object* obj,GenericPropertyInfo* property) {
            string str = property->get<string>(obj);
            char buffer[1000];
            std::strcpy(buffer,str.c_str());
            if(ImGui::InputText(label,buffer,1000)) {
                string s = buffer;
                property->set<string>(obj,s);
            }
        }
        
        void displayObjectReference(const char* label,Object* obj,GenericPropertyInfo* property) {
            Object* ref = property->getObj(obj);
            
            auto response = selector(label,(ref->name + " (" + ref->getTypeName() + ")"));
            if(response == SelectorResponse::Goto) {
                selectedObject = ref;
            }
        }

        void displaySprite(const char* label,Object* obj,GenericPropertyInfo* property) {
            Sprite sprite = property->get<Sprite>(obj);
            
            if(vulkan != nullptr) {
                auto info = vulkan->getTextureInfo(sprite.texture);
                displayImage(imageDisplaySizeSmall,vulkan->getImGuiTextureID(sprite.texture),info.width,info.height);
            }
            if(ImGui::TreeNode(label)) {
                displayTexture("texture",sprite.texture,false);
                displayRect("rect",sprite.rect);
                ImGui::TreePop();
            }

            property->set<Sprite>(obj,sprite);
            
            //ImGui::LabelText(label,"%s",sprite.name.c_str());
        }

        void displayMaterial(const char* label,Object* obj,GenericPropertyInfo* property,bool showImage = true) {
            Material material = property->get<Material>(obj);

            auto response = selector(label,material.name);
            if(response == SelectorResponse::Change) {
            }
            if(response == SelectorResponse::Goto) {
                auto data = registry->getMaterialData(material.name);
                if(data != nullptr) {
                    selectedObject = data;
                }
            }
        }

        
        void displayTexture(const char* label,TextureID& id,bool showImage = true) {
            
            if(vulkan != nullptr) {
                auto info = vulkan->getTextureInfo(id);
                if(showImage) displayImage(imageDisplaySizeSmall,vulkan->getImGuiTextureID(id),info.width,info.height);
                auto response = selector(label,info.name);
                if(response == SelectorResponse::Change) {
                }
                if(response == SelectorResponse::Goto) {
                    textureObjects[info.name].load(id,vulkan);
                    textureObjects[info.name].name = info.name;
                    selectedObject = &textureObjects[info.name];
                }
            } else {
                ImGui::LabelText(label,"%i",id);
            }
        }

        void displayTexture(const char* label,Object* obj,GenericPropertyInfo* property,bool showImage = true) {
            TextureID texture = property->get<TextureID>(obj);

            displayTexture(label,texture,showImage);
        }

        
        
        public:
            Registry* registry = nullptr;
            Vulkan* vulkan = nullptr;
            Object* selectedObject = nullptr;
            float imageDisplaySizeLarge = 300;
            float imageDisplaySizeSmall = 100;
            
            void display() {
                assert(registry != nullptr);

                if(iconFont == nullptr) {
                    ImGuiIO& io = ImGui::GetIO();
                    iconFont = io.Fonts->AddFontFromFileTTF("fonts\\lucide.ttf");
                }
                
                ImGui::Begin("Registry");
                
                ImGui::BeginTabBar("RegistryBar");
                
                tab("Items",registry->getItems());
                tab("Blocks",registry->getBlocks());
                tab("Recipes",registry->getRecipes());
                tab("Materials",registry->getMaterials());
                tab("Textures",registry->getTextures());
                tab("Models",registry->getModels());
                
                ImGui::EndTabBar();
                
                ImGui::End();
                
                    if(selectedObject != nullptr) {
                        inspector(selectedObject);
                    }
                    

                }

            

    };

}