#pragma once

#include <string>
#include <any>
#include "block/block.hpp"
#include "engine/object.hpp"
#include "interface/widgets-all.hpp"

using std::string;


template<typename T>
struct IsVector : public std::false_type {};

template<typename T>
struct IsVector<std::vector<T>> : public std::true_type {};

enum class PropertyTypeEnum {
    Int,
    Float,
    String,
    Bool,
    Enum,

    // Vec2,
    // IVec2,
    // Vec3,
    // IVec3,
    // Vec4,
    // Quat,
    Vector,
    Texture,
    Material,
    ObjectPointer,
    Pointer,
    Composite
};

struct GenericPropertyInfo {
    string name;
    GenericPropertyInfo(string name) : name(name) {}

    virtual int getEnumValue(std::any obj) const = 0;
    virtual int getEnumValue(Object* obj) const = 0;
    virtual std::any get(std::any obj) const = 0;
    virtual std::any get(Object* obj) const = 0;
    virtual std::any getPtr(std::any obj) const = 0;
    virtual std::any getPtr(Object* obj) const = 0;
    virtual Object* getObj(Object* obj) const = 0;
    virtual Object* getObj(std::any obj) const = 0;
    virtual void* getVoidPtr(std::any obj) const = 0; // for getting pointer type names
    virtual void* getVoidPtr(Object* obj) const = 0; // for getting pointer type names

    template<typename T>
    T get(std::any obj) const {
        return std::any_cast<T>(get(obj));
    }

    template<typename T>
    T get(Object* obj) const {
        return std::any_cast<T>(get(obj));
    }

    virtual void setEnumValue(std::any obj,int value) const = 0;
    virtual void setEnumValue(Object* obj,int value) const = 0;
    virtual void set(std::any obj,std::any value) const = 0;
    virtual void set(Object* obj,std::any value) const = 0;
    virtual void set(Object* obj,Object* value) const = 0;
    virtual void set(std::any obj,Object* value) const = 0;
    virtual void setPtr(std::any obj,std::any& value) const = 0;
    virtual void setPtr(Object* obj,std::any& value) const = 0;

    template<typename T>
    void set(std::any obj,T value) const {
        set(obj,std::make_any<T>(value));
    }

    template<typename T>
    void set(Object* obj,T value) const {
        set(obj,std::make_any<T>(value));
    }

    virtual string typeIdName() = 0;
    virtual string typeIdNameNonPointer() = 0;

    // 0 if no elements
    virtual int getSize(std::any obj) {
        return 0;
    }

    // 0 if no elements
    virtual int getSize(Object* obj) {
        return 0;
    }

    virtual void setSize(std::any obj,int size) {}

    virtual void setSize(Object* obj,int size) {}

    virtual void removeElement(std::any obj,int index) {}

    virtual void removeElement(Object* obj,int index) {}

    // null if has no elements
    virtual GenericPropertyInfo* getElement(int index) {
        return nullptr;
    }


    template<typename T>
    inline static PropertyTypeEnum getPropertyTypeGeneric() {
        if constexpr(std::is_convertible_v<T,Object*>) {
            return PropertyTypeEnum::ObjectPointer;
        }
        if constexpr(std::is_enum_v<T>) {
            return PropertyTypeEnum::Enum;
        }
        if constexpr(IsVector<T>::value) {
            return PropertyTypeEnum::Vector;
        }
        if constexpr(std::is_pointer_v<T>) {
            return PropertyTypeEnum::Pointer;
        }
        return PropertyTypeEnum::Composite;
    };
    template<typename T>
    inline static PropertyTypeEnum getPropertyElementTypeGeneric() {
        if constexpr(IsVector<T>::value) {
            return getPropertyTypeGeneric<typename T::value_type>();
        }
        return PropertyTypeEnum::Composite;
    };
    virtual PropertyTypeEnum getPropertyType() const = 0;
    virtual PropertyTypeEnum getPropertyElementType() const = 0;
    virtual ~GenericPropertyInfo() = default;
};

template<typename ClassType,typename PropertyType>
struct ElementPropertyInfo : GenericPropertyInfo {
    int index;
    ElementPropertyInfo(int index) : GenericPropertyInfo(std::to_string(index)), index(index) {}

    // virtual std::any get(std::any obj) {
    //     return obj.*property;
    // }

    std::any get(std::any obj) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        PropertyType value = typedObj->at(index);
        return std::make_any<PropertyType>(value);
    }

    std::any get(Object* obj) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        PropertyType value = typedObj->at(index);
        return std::make_any<PropertyType>(value);
    }

    std::any getPtr(std::any obj) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        PropertyType* value = &typedObj->at(index);
        return std::make_any<PropertyType*>(value);
    }

    std::any getPtr(Object* obj) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        PropertyType* value = &typedObj->at(index);
        return std::make_any<PropertyType*>(value);
    }

    Object* getObj(Object* obj) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->at(index);
            return value;
        } else {
            return nullptr;
        }
    }
    Object* getObj(std::any obj) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->at(index);
            return value;
        } else {
            return nullptr;
        }
    }

    void* getVoidPtr(std::any obj) const override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->at(index);
            return static_cast<void*>(value);
        } else {
            return nullptr;
        }
    }

    void* getVoidPtr(Object* obj) const override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->at(index);
            return static_cast<void*>(value);
        } else {
            return nullptr;
        }
    }

    void set(std::any obj,std::any value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        typedObj->at(index) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,std::any value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        typedObj->at(index) = std::any_cast<PropertyType>(value);
    }
    void setPtr(std::any obj,std::any& value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        if constexpr(std::is_pointer_v<PropertyType>) {
            typedObj->at(index) = std::any_cast<std::remove_pointer_t<PropertyType>>(&value);
        }
    }
    void setPtr(Object* obj,std::any& value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        if constexpr(std::is_pointer_v<PropertyType>) {
            typedObj->at(index) = std::any_cast<std::remove_pointer_t<PropertyType>>(&value);
        }
    }
    void set(Object* obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            if(typedValue == nullptr && value != nullptr) {
                Debug::warn("invalid type for property " + name);
                return;
            }
            typedObj->at(index) = typedValue;
        }
    }
    void set(std::any obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            if(typedValue == nullptr && value != nullptr) {
                Debug::warn("invalid type for property " + name);
                return;
            }
            typedObj->at(index) = typedValue;
        }
    }

    int getEnumValue(std::any obj) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            return (int)(typedObj->at(index));
        } else {
            return 0;
        }
    }
    
    int getEnumValue(Object* obj) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            return (int)(typedObj->at(index));
        } else {
            return 0;
        }
    }

    void setEnumValue(std::any obj,int value) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            (typedObj->at(index)) = value;
        }
    }

    void setEnumValue(Object* obj,int value) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            (typedObj->at(index)) = value;
        }
    }

    PropertyTypeEnum getPropertyType() const override {
        return getPropertyTypeGeneric<PropertyType>();
    }

    // for arrays
    PropertyTypeEnum getPropertyElementType() const override {
        return getPropertyElementTypeGeneric<PropertyType>();
    }

    string typeIdName() override {
        return typeid(PropertyType).name();
    }

    string typeIdNameNonPointer() override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            return typeid(std::remove_pointer_t<PropertyType>).name();
        }
        return typeid(PropertyType).name();
    }

};

template<typename ClassType,typename PropertyType>
struct PropertyInfo : GenericPropertyInfo {
    PropertyType ClassType::* property;
    PropertyInfo(string name, PropertyType ClassType::* property) : GenericPropertyInfo(name), property(property) {}

    std::map<int,std::unique_ptr<GenericPropertyInfo>> elementProperties;

    // virtual std::any get(std::any obj) {
    //     return obj.*property;
    // }

    std::any get(std::any obj) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        PropertyType value = typedObj->*(property);
        return std::make_any<PropertyType>(value);
    }

    std::any get(Object* obj) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        PropertyType value = typedObj->*(property);
        return std::make_any<PropertyType>(value);
    }

    std::any getPtr(std::any obj) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        PropertyType* value = &(typedObj->*(property));
        return std::make_any<PropertyType*>(value);
    }

    std::any getPtr(Object* obj) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        PropertyType* value = &(typedObj->*(property));
        return std::make_any<PropertyType*>(value);
    }

    Object* getObj(Object* obj) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->*(property);
            return value;
        } else {
            return nullptr;
        }
    }
    Object* getObj(std::any obj) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->*(property);
            return value;
        } else {
            return nullptr;
        }
    }

    void* getVoidPtr(std::any obj) const override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->*(property);
            return static_cast<void*>(value);
        } else {
            return nullptr;
        }
    }

    void* getVoidPtr(Object* obj) const override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType value = typedObj->*(property);
            return static_cast<void*>(value);
        } else {
            return nullptr;
        }
    }

    void set(std::any obj,std::any value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        typedObj->*(property) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,std::any value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        typedObj->*(property) = std::any_cast<PropertyType>(value);
    }
     void setPtr(std::any obj,std::any& value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        if constexpr(std::is_pointer_v<PropertyType>) {
            typedObj->*(property) = std::any_cast<std::remove_pointer_t<PropertyType>>(&value);
        }
     }
    
    void setPtr(Object* obj,std::any& value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        if constexpr(std::is_pointer_v<PropertyType>) {
            typedObj->*(property) = std::any_cast<std::remove_pointer_t<PropertyType>>(&value);
        }
    }
    void set(Object* obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            if(typedValue == nullptr && value != nullptr) {
                Debug::warn("invalid type for property " + name);
                return;
            }
            typedObj->*(property) = typedValue;
        }
    }
    void set(std::any obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            if(typedValue == nullptr && value != nullptr) {
                Debug::warn("invalid type for property " + name);
                return;
            }
            typedObj->*(property) = typedValue;
        }
    }
    

    GenericPropertyInfo* getElement(int index) override {
        if constexpr(IsVector<PropertyType>::value) {
            if(!elementProperties.contains(index)) {
                elementProperties[index] = std::make_unique<ElementPropertyInfo<PropertyType,typename PropertyType::value_type>>(index);
            }
            return elementProperties[index].get();
        }
        return nullptr;
    }

    int getSize(std::any obj) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            return (typedObj->*(property)).size();
        }
        return 0;
    }

    int getSize(Object* obj) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            return (typedObj->*(property)).size();
        }
        return 0;
    }
    
    void setSize(std::any obj,int size) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            (typedObj->*(property)).resize(size);
        }
    }

    void setSize(Object* obj,int size) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            (typedObj->*(property)).resize(size);
        }
    }

    void removeElement(std::any obj,int index) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            auto& array = (typedObj->*(property));
            array.erase(array.begin() + index);
        }
    }

    void removeElement(Object* obj,int index) override {
        if constexpr(IsVector<PropertyType>::value) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            auto& array = (typedObj->*(property));
            array.erase(array.begin() + index);
        }
    }

    int getEnumValue(std::any obj) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            return (int)(typedObj->*(property));
        } else {
            return 0;
        }
    }
    
    int getEnumValue(Object* obj) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            return (int)(typedObj->*(property));
        } else {
            return 0;
        }
    }

    void setEnumValue(std::any obj,int value) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            (typedObj->*(property)) = (PropertyType)value;
        }
    }

    void setEnumValue(Object* obj,int value) const override {
        if constexpr(std::is_enum_v<PropertyType>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            (typedObj->*(property)) = (PropertyType)value;
        }
    }
    



    PropertyTypeEnum getPropertyType() const override {
        return getPropertyTypeGeneric<PropertyType>();
    }

    // for arrays
    PropertyTypeEnum getPropertyElementType() const override {
        return getPropertyElementTypeGeneric<PropertyType>();
    }

    string typeIdName() override {
        return typeid(PropertyType).name();
    }

    string typeIdNameNonPointer() override {
        if constexpr(std::is_pointer_v<PropertyType>) {
            return typeid(std::remove_pointer_t<PropertyType>).name();
        }
        return typeid(PropertyType).name();
    }

};

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<bool>() {
    return PropertyTypeEnum::Bool;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<float>() {
    return PropertyTypeEnum::Float;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<string>() {
    return PropertyTypeEnum::String;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<int>() {
    return PropertyTypeEnum::Int;
}

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<vec3>() {
//     return PropertyTypeEnum::Vec3;
// }

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<ivec3>() {
//     return PropertyTypeEnum::IVec3;
// }

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<vec2>() {
//     return PropertyTypeEnum::Vec2;
// }

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<ivec2>() {
//     return PropertyTypeEnum::IVec2;
// }

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<quat>() {
//     return PropertyTypeEnum::Quat;
// }

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<Block*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<Item*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<TextWidget*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<ItemSlotWidget*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<InventoryWidget*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<ToolbarWidget*>() {
    return PropertyTypeEnum::ObjectPointer;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<PlayerWidget*>() {
    return PropertyTypeEnum::ObjectPointer;
}

// template<typename T*>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<T*>() {
//     return PropertyTypeEnum::ObjectPointer;
// }

// template<>
// inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<glm::vec4>() {
//     return PropertyTypeEnum::Vec4;
// }

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<TextureID>() {
    return PropertyTypeEnum::Texture;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<Material>() {
    return PropertyTypeEnum::Material;
}

template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<MaterialObject*>() {
    return PropertyTypeEnum::ObjectPointer;
}


template<>
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<Mesh<Vertex>*>() {
    return PropertyTypeEnum::ObjectPointer;
}



class TypeInfo {

    
    
    std::vector<std::unique_ptr<GenericPropertyInfo>> properties;
    std::map<string,GenericPropertyInfo*> propertyMap;
    
    string name = "";
    string folderName = "";
    string typeIdName = "";

    
    TypeInfo* parent = nullptr;
    
    public:
        std::function<std::unique_ptr<Object>()> constructorFunction = {};
        std::vector<TypeInfo*> derived;
        bool isEnum;
        std::vector<string> enumNames;
        TypeInfo(TypeInfo& typeinfo) = delete;
        TypeInfo() = default;
        TypeInfo(string name) : name(name) {}
        TypeInfo(string name,string typeIdName) : name(name), typeIdName(typeIdName) {}
        template<typename ClassType,typename PropertyType>
        GenericPropertyInfo* addConstProperty(string name, PropertyType ClassType::* property) {
            auto info = std::make_unique<PropertyInfo<ClassType,PropertyType>>(name,property);
            propertyMap[name] = info.get();
            properties.push_back(std::move(info));
            return propertyMap[name];
        }

        // void getProperty(string name) {

        // }

        TypeInfo* getParent() {
            return parent;
        }

        void setParent(TypeInfo* info) {
            Debug::addTrace(name);
            if(parent != nullptr) {
                Debug::warn("a parent has already been set");
            }
            parent = info;
            info->addDerviedType(this);
            Debug::subtractTrace();
        }

        void addDerviedType(TypeInfo* info) {
            derived.push_back(info);
            if(parent != nullptr) {
                parent->addDerviedType(info);
            }
        }
        

        string getName() {
            return name;
        }

        string getTypeId() {
            return typeIdName;
        }

        string getFolderName() {
            if(folderName != "") {
                return folderName;
            }
            if(parent == nullptr) {
                return name + "s";
            } else {
                return parent->getFolderName();
            }
        }

        void setFolderName(string name) {
            folderName = name;
        }

        string getRootName() {
            if(parent == nullptr) {
                return name;
            } else {
                return parent->getRootName();
            }
        }

        const std::vector<std::unique_ptr<GenericPropertyInfo>>& getProperties() {
            return properties;
        }

};


// #define TYPE_INFO(typeName) \
//     TypeInfo* _typeName = registry.addTypeInfo<typeName>(#typeName); \
//     // set CURRENT_TYPEINFO = typeName


// #define TYPE_INFO_PROPERTY(propertyName) \
//     CURRENT_TYPE_INFO->addConstProperty(#propertyName, &CURRENT_TYPE_INFO::propertyName);