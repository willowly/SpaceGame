#pragma once

#include <string>
#include <any>
#include "block/block.hpp"
#include "engine/object.hpp"

using std::string;


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

    Texture,
    Material,
    ObjectPointer,
    Complex
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



    template<typename T>
    inline static PropertyTypeEnum getPropertyTypeGeneric() {
        return PropertyTypeEnum::Complex;
    }
    virtual PropertyTypeEnum getPropertyType() const = 0;
    virtual ~GenericPropertyInfo() = default;
};

template<typename ClassType,typename PropertyType>
struct PropertyInfo : GenericPropertyInfo {
    PropertyType ClassType::* property;
    PropertyInfo(string name, PropertyType ClassType::* property) : GenericPropertyInfo(name), property(property) {}

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


    PropertyTypeEnum getPropertyType() const override {
        return getPropertyTypeGeneric<PropertyType>();
    }

    string typeIdName() override {
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
inline PropertyTypeEnum GenericPropertyInfo::getPropertyTypeGeneric<Mesh<Vertex>*>() {
    return PropertyTypeEnum::ObjectPointer;
}



class TypeInfo {

    struct AnyTypeBase {
        virtual ~AnyTypeBase() = default;
    };

    template<typename T> 
    struct AnyType : AnyTypeBase {
        T* pointer;
    };
    
    
    std::vector<std::unique_ptr<GenericPropertyInfo>> properties;
    std::map<string,GenericPropertyInfo*> propertyMap;
    
    string name = "";
    
    
    public:
        TypeInfo* parent = nullptr;
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

        void getProperty(string name) {

        }

        string getName() {
            return name;
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