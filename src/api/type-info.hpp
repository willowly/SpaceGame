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
    Composite
};

struct GenericPropertyInfo {
    string name;
    GenericPropertyInfo(string name) : name(name) {}

    virtual std::any get(std::any obj) const = 0;
    virtual std::any get(Object* obj) const = 0;
    virtual std::any getPtr(std::any obj) const = 0;
    virtual std::any getPtr(Object* obj) const = 0;
    virtual Object* getObj(Object* obj) const = 0;
    virtual Object* getObj(std::any obj) const = 0;

    template<typename T>
    T get(std::any obj) const {
        return std::any_cast<T>(get(obj));
    }

    template<typename T>
    T get(Object* obj) const {
        return std::any_cast<T>(get(obj));
    }


    virtual void set(std::any obj,std::any value) const = 0;
    virtual void set(Object* obj,std::any value) const = 0;
    virtual void set(Object* obj,Object* value) const = 0;
    virtual void set(std::any obj,Object* value) const = 0;

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
        if constexpr(IsVector<T>::value) {
            return PropertyTypeEnum::Vector;
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

    void set(std::any obj,std::any value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        typedObj->at(index) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,std::any value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        typedObj->at(index) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            assert(typedValue != nullptr);
            typedObj->at(index) = typedValue;
        }
    }
    void set(std::any obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            assert(typedValue != nullptr);
            typedObj->at(index) = typedValue;
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

    void set(std::any obj,std::any value) const override {
        ClassType* typedObj = std::any_cast<ClassType*>(obj);
        typedObj->*(property) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,std::any value) const override {
        ClassType* typedObj = dynamic_cast<ClassType*>(obj);
        assert(typedObj != nullptr);
        typedObj->*(property) = std::any_cast<PropertyType>(value);
    }
    void set(Object* obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = dynamic_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            assert(typedValue != nullptr);
            typedObj->*(property) = typedValue;
        }
    }
    void set(std::any obj,Object* value) const override {
        if constexpr(std::is_convertible_v<PropertyType,Object*>) {
            ClassType* typedObj = std::any_cast<ClassType*>(obj);
            assert(typedObj != nullptr);
            PropertyType typedValue = dynamic_cast<PropertyType>(value);
            assert(typedValue != nullptr);
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

    
    TypeInfo* parent = nullptr;
    
    public:
        std::function<std::unique_ptr<Object>()> constructorFunction = {};
        std::vector<TypeInfo*> derived;
        TypeInfo(TypeInfo& typeinfo) = delete;
        TypeInfo() = default;
        TypeInfo(string name) : name(name) {}
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
            if(parent != nullptr) {
                Debug::warn("a parent has already been set");
            }
            parent = info;
            info->addDerviedType(this);
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