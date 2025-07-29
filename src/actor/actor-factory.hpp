#pragma once
#include "actors-all.hpp"
#include "actor-factory-fwd.hpp"

template<> 
std::unique_ptr<Actor> ActorFactory::make(Actor* prototype,vec3 position,quat rotation) {

    return makeFromPrototype(prototype,position,rotation);
}

template<> 
std::unique_ptr<Actor> ActorFactory::make(Actor* prototype,vec3 position) {

    return makeFromPrototype(prototype,position,glm::identity<quat>());
}

template<> 
std::unique_ptr<Character> ActorFactory::make(Character* prototype,vec3 position,quat rotation) {

    return makeFromPrototype(prototype,position,rotation);
}

template<> 
std::unique_ptr<Character> ActorFactory::make(Character* prototype,vec3 position) {

    return makeFromPrototype(prototype,position,glm::identity<quat>());
}

template<> 
std::unique_ptr<RigidbodyActor> ActorFactory::make(RigidbodyActor* prototype,vec3 position,quat rotation) {

    return makeFromPrototype(prototype,position,rotation);
}

template<> 
std::unique_ptr<RigidbodyActor> ActorFactory::make(RigidbodyActor* prototype,vec3 position) {

    return makeFromPrototype(prototype,position,glm::identity<quat>());
}

template<> 
std::unique_ptr<Construction> ActorFactory::make(vec3 position,quat rotation) {

    auto newActor = std::make_unique<Construction>();
    newActor->position = position;
    newActor->rotation = rotation;
    return newActor;
}

template<> 
std::unique_ptr<Construction> ActorFactory::make(vec3 position) {

    return ActorFactory::make<Construction>(position,glm::identity<quat>());

}


template<> 
std::unique_ptr<Terrain> ActorFactory::make(Material* material,vec3 position) {

    auto newActor = std::make_unique<Terrain>();
    newActor->material = material;
    newActor->position = position;
    return newActor;

}


