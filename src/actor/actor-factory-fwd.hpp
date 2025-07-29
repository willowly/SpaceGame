#pragma once
#include "actor.hpp"
#include <engine/debug.hpp>

namespace ActorFactory {

    template<typename T,typename... Args>
    std::unique_ptr<T> make(Args... args) = delete;

    template<typename T>
    std::unique_ptr<T> makeFromPrototype(T* prototype) {
        std::unique_ptr<T> newActor = nullptr;
        if(prototype == nullptr) {
            Debug::warn("tried to make an actor with a null prototype");
            newActor = std::make_unique<T>();
        } else {
            newActor = std::make_unique<T>(*prototype);
        }

        return newActor;
    }

    template<typename T>
    std::unique_ptr<T> makeFromPrototype(T* prototype,vec3 position,quat rotation = glm::identity<quat>()) {

        std::unique_ptr<T> newActor = makeFromPrototype<T>(prototype);
        newActor->position = position;
        newActor->rotation = rotation;

        return newActor;

    }
    
}