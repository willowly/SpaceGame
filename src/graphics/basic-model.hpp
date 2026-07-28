#pragma once
#include "mesh.hpp"
#include "vulkan.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

using glm::vec3, glm::quat;

struct BasicModel {
    Mesh<Vertex>* mesh = nullptr;
    MaterialObject* material = nullptr;

    vec3 offset = {};
    quat rotation = {};
    float scale = 1;

    void addRenderables(Vulkan* vulkan,vec3 position,quat rotation) {
        if(mesh == nullptr) return;
        if(material == nullptr) return;
        mesh->addToRender(vulkan,material->material,position + rotation*offset,rotation * this->rotation,vec3(scale));
    }
};