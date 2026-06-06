#include "type-info.hpp"

#include <iostream>

class AssetSerializer {

    int indents = 0;

    template<typename PropertyType,typename T>
    void serializeNumber(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << std::to_string(value);
    }

    template<typename PropertyType,typename T>
    void serializeBasic(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << value;
    }

    template<typename PropertyType,typename T>
    void serializeVector(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << StringHelper::toString(value);
    }

    template<typename T>
    void serializeTexture(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto id = property->get<TextureID>(obj);
        file << vulkan->getTextureInfo(id).name;
    }

    template<typename T>
    void serializeMaterial(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto mat = property->get<Material>(obj);
        file << mat.name;
    }

    template<typename T>
    void serializeObjectPointer(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto pointer = property->getObj(obj);
        file << pointer->name;
    }

    template<typename PropertyType,typename T>
    void serializeComposite(T obj,GenericPropertyInfo* property,string typeName,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << "\n";
        indents++;
        TypeInfo* info = registry->getTypeInfo(typeName);
        if(info != nullptr) {
            serialize(&value,info,file); //this needs to be a pointer
        }
        indents--;
    }

    template<typename T>
    void serializeComposite(T obj,GenericPropertyInfo* property,string typeName,ofstream& file) {
        auto value = property->getPtr(obj);
        file << "\n";
        indents++;
        TypeInfo* info = registry->getTypeInfo(typeName);
        if(info != nullptr) {
            serialize(value,info,file); //this needs to be a pointer
        }
        indents--;
    }

    template<typename T>
    void serializeProperty(T obj,GenericPropertyInfo* property,ofstream& file) {
        bool newLine = true;
        for (size_t i = 0; i < indents; i++)
        {
            file << "  ";
        }
        file << property->name + ": ";
        
        switch(property->getPropertyType()) {
            case PropertyTypeEnum::Int:
                serializeNumber<int>(obj,property,file);
                break;
            case PropertyTypeEnum::Float:
                serializeNumber<float>(obj,property,file);
                break;
            case PropertyTypeEnum::String:
                serializeBasic<string>(obj,property,file);
                break;
            // case PropertyTypeEnum::Vec2:
            //     serializeVector<vec2>(obj,property,file);
            //     break;
            // case PropertyTypeEnum::IVec2:
            //     serializeVector<ivec2>(obj,property,file);
            //     break;
            // case PropertyTypeEnum::Vec3:
            //     serializeVector<vec3>(obj,property,file);
            //     break;
            // case PropertyTypeEnum::IVec3:
            //     serializeVector<ivec3>(obj,property,file);
            //     break;
            // case PropertyTypeEnum::Vec4:
            //     serializeVector<vec4>(obj,property,file);
            //     break;
            // case PropertyTypeEnum::Quat:
            //     serializeVector<quat>(obj,property,file);
            //     break;
            case PropertyTypeEnum::Bool:
                serializeBasic<bool>(obj,property,file);
                break;
            case PropertyTypeEnum::Texture:
                serializeTexture(obj,property,file);
                break;
            case PropertyTypeEnum::Material:
                serializeMaterial(obj,property,file);
                break;
            case PropertyTypeEnum::ObjectPointer:
                serializeObjectPointer(obj,property,file);
                break;
            case PropertyTypeEnum::Complex:
                serializeComposite(obj,property,registry->getTypeName(property->typeIdName()),file);
                newLine = false;
                break;      
        }
        if(newLine) {
            file << "\n";
        }
    }

    public:
        Registry* registry = nullptr;
        Vulkan* vulkan = nullptr;
        template<typename T>
        void serialize(T obj,TypeInfo* info,ofstream& file) {
            assert(registry != nullptr);
            assert(vulkan != nullptr);
            assert(info != nullptr);
            for(auto& property : info->getProperties()) {
                serializeProperty(obj,property.get(),file);
            }
            if(info->parent != nullptr) {
                serialize(obj,info->parent,file);
            }
        }




};