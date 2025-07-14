#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include <reactphysics3d/reactphysics3d.h>
#include <graphics/model.hpp>


namespace PhysicsHelper {

    rp3d::Vector3 toRp3dVector(glm::vec3 v) {
        return rp3d::Vector3(v.x,v.y,v.z);
    }

    glm::vec3 toGlmVector(rp3d::Vector3 v) {
        return glm::vec3(v.x,v.y,v.z);
    }

    rp3d::Quaternion toRp3dQuaternion(glm::quat q) {
        return rp3d::Quaternion(q.x,q.y,q.z,q.w);
    }

    glm::quat toGlmQuaternion(rp3d::Quaternion q) {
        return glm::quat(q.w,q.x,q.y,q.z);
    }

    rp3d::TriangleMesh* applyTriangleMesh(rp3d::PhysicsCommon* common,Model* model) {
        int vertexCount = 3 * model->vertices.size();
        float vertices[vertexCount];
        int i = 0;
        for(vec3 vertex : model->vertices) {
            vertices[i] = vertex.x;
            vertices[i+1] = vertex.y;
            vertices[i+2] = vertex.z;
            i += 3;
        }
        i = 0;
        int indexCount = 3 * model->faces.size();
        int indices[indexCount];
        for(Model::Face& face : model->faces) {
            indices[i] = face.vertexIndices[0];
            indices[i+1] = face.vertexIndices[1];
            indices[i+2] = face.vertexIndices[2];
            i += 3;
        }
        rp3d::TriangleVertexArray triangleArray = rp3d::TriangleVertexArray(vertexCount, vertices, 3 * sizeof(float), indexCount,indices, 3 * sizeof(int),rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
        
        std::vector<rp3d::Message> messages;
        common->createBoxShape(rp3d::Vector3(1,1,1));
        rp3d::TriangleMesh* triangleMesh = common->createTriangleMesh(triangleArray, messages);
        
        // Display the messages (info, warning and errors)
        if (messages.size() > 0) {
        
            for (const rp3d::Message& message: messages) {
        
                std::string messageType;
                switch(message.type) {
                    case rp3d::Message::Type::Information:
                        messageType = "info";
                        break;
                    case rp3d::Message::Type::Warning:
                        messageType = "warning";
                        break;
                    case rp3d::Message::Type::Error:
                        messageType = "error";
                        break;
                }
        
                std::cout << "Message (" << messageType << "): " << message.text << std::endl;
            }
        }

        return triangleMesh;
    }

    struct RaycastCallback : public rp3d::RaycastCallback {

        public:

            bool success = false;
            glm::vec3 worldPoint;
            glm::vec3 worldNormal;
            rp3d::Body* body;


            rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) {

                
                success= true;

                worldPoint = toGlmVector(info.worldPoint);
                worldNormal = toGlmVector(info.worldNormal);
                body = info.body;

                
                return info.hitFraction;
            }
    };
}

namespace ph = PhysicsHelper;