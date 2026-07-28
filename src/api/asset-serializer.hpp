#pragma once
#include "type-info.hpp"

#include <iostream>

class AssetSerializer {

    

    template<typename PropertyType,typename T>
    void serializeNumber(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << std::to_string(value);
    }

    template<typename T>
    void serializeEnum(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->getEnumValue(obj);
        file << std::to_string(value);
    }

    template<typename PropertyType,typename T>
    void serializeBasic(T obj,GenericPropertyInfo* property,ofstream& file) {
        auto value = property->get<PropertyType>(obj);
        file << value;
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
        if(pointer != nullptr) {
            file << pointer->name;
        }
    }

    template<typename PropertyType,typename T>
    void serializeComposite(T obj,GenericPropertyInfo* property,string typeName,ofstream& file,int indents = 0) {
        auto value = property->get<PropertyType>(obj);
        file << "\n";
        TypeInfo* info = registry->getTypeInfo(typeName);
        if(info != nullptr) {
            serialize(&value,info,file,indents+1); //this needs to be a pointer
        }
    }

    template<typename T>
    void serializeComposite(T obj,GenericPropertyInfo* property,string typeName,ofstream& file,int indents = 0) {
        auto value = property->getPtr(obj);
        file << "\n";
        TypeInfo* info = registry->getTypeInfo(typeName);
        if(info != nullptr) {
            serialize(value,info,file,indents+1); //this needs to be a pointer
        }
    }

    template<typename T>
    void serializeVector(T obj,GenericPropertyInfo* property,ofstream& file,int indents = 0) {
        auto value = property->getPtr(obj);
        file << "\n";
        for (size_t i = 0; i < property->getSize(obj); i++)
        {
            serializeProperty(value,property->getElement(i),file,indents+1);
        }
        
    }
    

    template<typename T>
    void serializeProperty(T obj,GenericPropertyInfo* property,ofstream& file,int indents = 0) {
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
            case PropertyTypeEnum::Enum:
                serializeEnum(obj,property,file);
                break;
            case PropertyTypeEnum::Float:
                serializeNumber<float>(obj,property,file);
                break;
            case PropertyTypeEnum::String:
                serializeBasic<string>(obj,property,file);
                break;
            case PropertyTypeEnum::Bool:
                serializeBasic<bool>(obj,property,file);
                break;
            case PropertyTypeEnum::Texture:
                serializeTexture(obj,property,file);
                break;
            case PropertyTypeEnum::Material:
                serializeMaterial(obj,property,file);
                break;
            case PropertyTypeEnum::Vector:
                serializeVector(obj,property,file,indents);
                newLine = false;
                break;
            case PropertyTypeEnum::ObjectPointer:
                serializeObjectPointer(obj,property,file);
                break;
            case PropertyTypeEnum::Composite:
                serializeComposite(obj,property,registry->getTypeName(property->typeIdName()),file,indents);
                newLine = false;
                break;      
        }
        if(newLine) {
            file << "\n";
        }
    }

    struct SerializationNode {
        std::variant<string,map<string,SerializationNode>> value;

        string getValue() {
            if(value.index() == 0) {
                return std::get<string>(value);
            } else {
                return "";
            }
        }
    };

    template<typename T>
    void deserializeInt(T obj,GenericPropertyInfo* property,SerializationNode node) {
        int i = 0;
        try {
            i = std::stoi(node.getValue());
        } catch (std::invalid_argument& e) {
            Debug::warn("not an int! (expected an int)");
        }
        property->set(obj,i);
    }

    template<typename T>
    void deserializeEnum(T obj,GenericPropertyInfo* property,SerializationNode node) {
        int i = 0;
        try {
            i = std::stoi(node.getValue());
        } catch (std::invalid_argument& e) {
            Debug::warn("not an int! (expected an int)");
        }
        property->setEnumValue(obj,i);
    }
    template<typename T>
    void deserializeFloat(T obj,GenericPropertyInfo* property,SerializationNode node) {
        float f = 0;
        try {
            f = std::stof(node.getValue());
        } catch (std::invalid_argument& e) {
            Debug::warn("not a float! (expected an float)");
        }
        property->set(obj,f);
    }
    template<typename T>
    void deserializeBool(T obj,GenericPropertyInfo* property,SerializationNode node) {
        int i = 0;
        try {
            i = std::stoi(node.getValue());
        } catch (std::invalid_argument& e) {
            Debug::warn("not an int! (expected an int)");
        }
        property->set(obj,i != 0);
    }
    template<typename T>
    void deserializeString(T obj,GenericPropertyInfo* property,SerializationNode node) {
        property->set(obj,node.getValue());
    }

    template<typename T>
    void deserializeTexture(T obj,GenericPropertyInfo* property,SerializationNode node) {
        property->set(obj,registry->getTexture(node.getValue()));
    }

    template<typename T>
    void deserializeMaterial(T obj,GenericPropertyInfo* property,SerializationNode node) {
        property->set(obj,registry->getMaterial(node.getValue()));
    }

    template<typename T>
    void deserializeObjectPointer(T obj,GenericPropertyInfo* property,SerializationNode node) {

        string typeName = registry->getTypeName(property->typeIdNameNonPointer());
        if(typeName == "item") {
            property->set(obj,registry->getItem(node.getValue()));
            return;
        }
        if(typeName == "block") {
            property->set(obj,registry->getBlock(node.getValue()));
            return;
        }
        if(typeName == "actor") {
            property->set(obj,registry->getActor(node.getValue()));
            return;
        }
        if(typeName == "recipe") {
            property->set(obj,registry->getRecipe(node.getValue()));
            return;
        }
        if(typeName == "material_object") {
            property->set(obj,registry->getPtr<MaterialObject>(node.getValue()));
            return;
        }
        if(typeName == "particleEffect") {
            property->set(obj,registry->getParticleEffect(node.getValue()));
            return;
        }
        if(typeName == "mesh") {
            property->set(obj,registry->getModel(node.getValue()));
            return;
        }
    }

    template<typename T>
    void deserializeComposite(T obj,GenericPropertyInfo* property,SerializationNode node) {
        TypeInfo* typeInfo = registry->getTypeInfo(registry->getTypeName(property->typeIdName()));
        if(typeInfo == nullptr) {
            Debug::warn("no type info for " + property->typeIdName());
            return;
        }
        if(node.value.index() != 1) {
            Debug::warn("serialization node is not a map for " + property->typeIdName());
            return;
        }
        auto value = property->getPtr(obj);
        deserializeProperties(value,typeInfo,std::get<map<string,SerializationNode>>(node.value));
    }

    template<typename T>
    void deserializeVector(T obj,GenericPropertyInfo* property,SerializationNode node) {
        if(node.value.index() != 1) {
            Debug::warn("serialization node is not a map for " + property->typeIdName());
            return;
        }
        auto value = property->getPtr(obj);
        auto map = std::get<std::map<string,SerializationNode>>(node.value);
        property->setSize(obj,map.size());
        for (size_t i = 0; i < map.size(); i++)
        {
            deserializeProperty(value,property->getElement(i),map.at(std::to_string(i)));
        }
        
    }
    
    std::map<string,SerializationNode> getNodeMap(ifstream& file,int indents = 0) {
        std::map<string,SerializationNode> map;

        auto lastPosition = file.tellg();
        string line;
        while(std::getline(file,line)) {
            auto tokens = StringHelper::split(line,":",1);
            if(tokens.size() != 2) {
                break;
            }
            string key = tokens[0];
            string value = tokens[1];
            StringHelper::trim(value);
            if(key.find_first_not_of(" ") < indents*2) {
                file.seekg(lastPosition);
                return map;
            }
            if(value != "") {
                StringHelper::trim(key);
                map[key] = SerializationNode(value);
            } else {
                StringHelper::trim(key);
                map[key] = SerializationNode(getNodeMap(file,indents+1));
            }
            lastPosition = file.tellg();
        }
        return map;
    }

    template<typename T>
    void deserializeProperties(T obj,TypeInfo* info,std::map<string,SerializationNode> map) {
        
        if(info == nullptr) {
            Debug::warn("Couldn't deserialize properties, typeinfo null");
            return;
        }
            
        for(auto& property : info->getProperties()) {
            if(map.contains(property->name)) {
                deserializeProperty(obj,property.get(),map[property->name]);
            }
        }
        if(info->getParent() != nullptr) {
            deserializeProperties(obj,info->getParent(),map);
        }

    }
    template<typename T>
    void deserializeProperty(T obj,GenericPropertyInfo* property,SerializationNode node) {
        
        switch(property->getPropertyType()) {
            case PropertyTypeEnum::Int:
                deserializeInt(obj,property,node);
                break;
            case PropertyTypeEnum::Enum:
                deserializeEnum(obj,property,node);
                break;
            case PropertyTypeEnum::Float:
                deserializeFloat(obj,property,node);
                break;
            case PropertyTypeEnum::String:
                deserializeString(obj,property,node);
                break;
            case PropertyTypeEnum::Bool:
                deserializeBool(obj,property,node);
                break;
            case PropertyTypeEnum::Texture:
                deserializeTexture(obj,property,node);
                break;
            case PropertyTypeEnum::Material:
                deserializeMaterial(obj,property,node);
                break;
            case PropertyTypeEnum::ObjectPointer:
                deserializeObjectPointer(obj,property,node);
                break;
            case PropertyTypeEnum::Vector:
                deserializeVector(obj,property,node);
                break; 
            case PropertyTypeEnum::Composite:
                deserializeComposite(obj,property,node);
                break;      
        }
    }

    TypeInfo* getTypeInfo(std::map<string,SerializationNode> map) {
        if(!map.contains("type")) return nullptr;
            
        auto typeInfo = registry->getTypeInfo(map.at("type").getValue());

        return typeInfo;
    }

    template<typename T>
        std::unique_ptr<T> deserializeStub(ifstream& file,TypeInfo* typeInfo,std::map<string,SerializationNode>& map) {

            assert(registry != nullptr);
            assert(vulkan != nullptr);

            if(typeInfo == nullptr) {
                Debug::warn("Couldn't deserialize file, typeinfo null");
                return nullptr;
            }

            // if(std::empty(typeInfo->constructorFunction)) {
            //     Debug::warn("Couldn't deserialize file, no object constructor");
            // }

            auto uniquePtrObj = typeInfo->constructorFunction();

            auto rawTypedPtr = dynamic_cast<T*>(uniquePtrObj.get());
            uniquePtrObj.release();

            auto uniqueTypedPtr = std::unique_ptr<T>(rawTypedPtr);

            if(uniqueTypedPtr == nullptr) {
                return nullptr;
            }

            return uniqueTypedPtr;

        }

    public:
        Registry* registry = nullptr;
        Vulkan* vulkan = nullptr;
        template<typename T>
        void serialize(T obj,TypeInfo* info,ofstream& file,int indents = 0,bool addType = false) {
            assert(registry != nullptr);
            assert(vulkan != nullptr);
            assert(info != nullptr);
            if(addType) {
                for (size_t i = 0; i < indents; i++)
                {
                    file << "  ";
                }
                file << "type: ";
                file << info->getName() + "\n";
            }
            for(auto& property : info->getProperties()) {
                serializeProperty(obj,property.get(),file,indents);
            }
            if(info->getParent() != nullptr) {
                serialize(obj,info->getParent(),file,indents);
            }
        }

        template<typename T>
        std::unique_ptr<T> deserialize(ifstream& file) {
            assert(registry != nullptr);
            assert(vulkan != nullptr);

            auto map = getNodeMap(file);

            TypeInfo* typeInfo = getTypeInfo(map);

            if(typeInfo == nullptr) {
                return nullptr;
            }

            auto uniqueTypedPtr = deserializeStub<T>(file,typeInfo,map);

            if(uniqueTypedPtr == nullptr) {
                return nullptr;
            }

            deserializeProperties(uniqueTypedPtr.get(),typeInfo,map);
            //registry->getTypeInfo()
            return uniqueTypedPtr;
        }

        template<typename T>
        std::unique_ptr<T> deserializeStub(ifstream& file) {

            assert(registry != nullptr);
            assert(vulkan != nullptr);

            auto map = getNodeMap(file);

            TypeInfo* typeInfo = getTypeInfo(map);

            if(typeInfo == nullptr) {
                return nullptr;
            }

            return deserializeStub<T>(file,typeInfo,map);

        }

        template<typename T>
        void deserializeProperties(T obj,ifstream& file) {

            assert(registry != nullptr);
            assert(vulkan != nullptr);

            auto map = getNodeMap(file);

            TypeInfo* typeInfo = getTypeInfo(map);

            if(typeInfo == nullptr) {
                return;
            }
            
            deserializeProperties<T>(obj,typeInfo,map);

        }

        




};