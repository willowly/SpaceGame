#pragma once

#include "actor.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

#include "engine/debug.hpp"
#include "physics/resolution.hpp"
#include "physics/intersections.hpp"

#include <helper/string-helper.hpp>
#include <algorithm>

#include <block/block.hpp>
#include <block/block-state.hpp>

#include "helper/block-storage.hpp"

using glm::ivec3,glm::vec3;
using std::unordered_map;

struct ConstructionVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 texCoord;
    int textureID;

    ConstructionVertex() {}

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ConstructionVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ConstructionVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ConstructionVertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(ConstructionVertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32_SINT;
        attributeDescriptions[3].offset = offsetof(ConstructionVertex, textureID);

        return attributeDescriptions;
    }

    ConstructionVertex(Vertex vertex,TextureID texture) {
        pos = vertex.pos;
        normal = vertex.normal;
        texCoord = vertex.texCoord;
        textureID = texture;

    }


};

#define CONSTRUCTION
class Construction : public Actor {


    struct BlockData {
        Block* block = nullptr; //eventually this will be a list of blocks in the construction
        BlockState state;
        BlockData(Block* block,BlockState state) : block(block), state(state) {}
        BlockData() {}
    };

    ivec3 min = ivec3(0,0,0);
    ivec3 max = ivec3(0,0,0);
    std::vector<BlockData> blockData;
    std::vector<ConstructionVertex> vertices;
    std::vector<uint16_t> indices;

    Material material = Material::none;

    int meshState = -1;
    MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
    bool gpuMeshOutOfDate = false;
    //bool meshOutOfDate;

    std::vector<int> blockCountX;

    vec3 moveControl;
    quat targetRotation = glm::identity<quat>();
    float turnForce = 1.5;
    vec3 velocity;

    int blockCount; //block count

    Construction() : Actor() {
        blockData.push_back(BlockData()); //starting block
        blockCountX.push_back(0);
    }

    
    public:
        // USES FACING AS INDEX
        // 0 - FORWARD
        // 1 - BACKWARD
        // 2 - UP,
        // 3 - DOWN
        // 4 - LEFT
        // 5 - RIGHT
        float thrustForces[6];


        static BlockFacing getFacingFromVector(vec3 v) {
            v = glm::normalize(v);
            if(v.z == 1) {
                return BlockFacing::FORWARD;
            }
            if(v.z == -1) {
                return BlockFacing::BACKWARD;
            }
            if(v.y == 1) {
                return BlockFacing::UP;
            }
            if(v.y == -1) {
                return BlockFacing::DOWN;
            }
            if(v.x == 1) {
                return BlockFacing::RIGHT;
            }
            if(v.x == -1) {
                return BlockFacing::LEFT;
            }
            return BlockFacing::FORWARD;
        }

        static quat getRotationFromFacing(BlockFacing blockFacing) {
            switch(blockFacing) {
                case BlockFacing::FORWARD:
                    return glm::angleAxis(glm::radians(0.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::BACKWARD:
                    return glm::angleAxis(glm::radians(180.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::UP:
                    return glm::angleAxis(glm::radians(-90.0f),vec3(1.0f,0,0));
                    break;
                case BlockFacing::DOWN:
                    return glm::angleAxis(glm::radians(90.0f),vec3(1.0f,0,0));
                    break;
                case BlockFacing::RIGHT:
                    return glm::angleAxis(glm::radians(90.0f),vec3(0,1.0f,0));
                    break;
                case BlockFacing::LEFT:
                    return glm::angleAxis(glm::radians(-90.0f),vec3(0,1.0f,0));
                    break;
            }
        }

        // idk
        struct Location {
            int x = 0;
            int y = 0;
            int z = 0;
            Location(int x,int y,int z) : x(x), y(y), z(z) {

            }
            Location(ivec3 i) : x(i.x), y(i.y), z(i.z) {
                
            }
            ivec3 asVec3() const {
                return ivec3(x,y,z);
            }

            constexpr auto operator<=>(const Location&) const = default;

        };

        std::map<Location,BlockStorage> blockStorage;
        std::map<Location,bool> stepCallbacks;

        void step(World* world,float dt) {
            
            for(auto& pair : stepCallbacks) {

                ivec3 location = pair.first.asVec3();
                bool enabled = pair.second;
                if(enabled) {

                    Block* block;
                    BlockState blockState;
                    std::tie(block,blockState) = getBlock(location);

                    if(block != nullptr) {
                        block->onStep(world,this,location,blockState,dt);
                    }
                }
            }

            //i have no idea why z and x are inverted :shrug:
            if(moveControl.z > 0) {
                applyForce(vec3(0,0,-1) * thrustForces[BlockFacing::FORWARD] * moveControl.z * dt);
            }
            if(moveControl.z < 0) {
                applyForce(vec3(0,0,-1) * thrustForces[BlockFacing::BACKWARD] * moveControl.z * dt);
            }
            if(moveControl.x < 0) {
                applyForce(vec3(-1,0,0) * thrustForces[BlockFacing::LEFT] * moveControl.x * dt);
            }
            if(moveControl.x > 0) {
                applyForce(vec3(-1,0,0) * thrustForces[BlockFacing::RIGHT] * moveControl.x * dt);
            }
            if(moveControl.y < 0) {
                applyForce(vec3(0,1,0) * thrustForces[BlockFacing::UP] * moveControl.y * dt);
            }
            if(moveControl.y > 0) {
                applyForce(vec3(0,1,0) * thrustForces[BlockFacing::DOWN] * moveControl.y * dt);
            }
            //velocity = MathHelper::moveTowards(velocity,rotation * targetVelocity,thrustForce * dt);
            rotation = glm::slerp(rotation,targetRotation,turnForce * dt);
            position += velocity * dt;
            
        }

        void applyForce(vec3 force) {
            velocity += transformDirection(force);
        }

        void generateMesh() {

            //if(!meshOutOfDate) return;

            vertices.clear();
            indices.clear();

            size_t i = 0;
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        
                        if(blockData.size() > i && blockData[i].block != nullptr) {
                            quat rotation = quat(vec3(0.0));//getRotationFromFacing(BlockFacing::FORWARD);
                            
                            auto block = blockData[i].block;
                            switch(block->modelType) {
                                case Block::ModelType::Mesh:
                                    addMeshBlock(vec3(x,y,z),rotation,block);
                                    break;
                                case Block::ModelType::SingleBlock:
                                    addSingleBlock(vec3(x,y,z),rotation,block);
                                    break;
                                case Block::ModelType::ConnectedBlock:
                                    addSingleBlock(vec3(x,y,z),rotation,block);
                                    break;
                            }
                            
                        }
                        i++;
                    }
                }
            }

            gpuMeshOutOfDate = true;


        }

        void addBlockFace(vec3 position,quat rotation,TextureID textureID) {
            int indexOffset = vertices.size();
            auto faceVerts = vector<ConstructionVertex> {
                ConstructionVertex(Vertex(vec3(0.5,0.5,0.5),vec3(0.0,0.0,1.0),vec2(0,0)),textureID),
                ConstructionVertex(Vertex(vec3(0.5,-0.5,0.5),vec3(0.0,0.0,1.0),vec2(0,1)),textureID),
                ConstructionVertex(Vertex(vec3(-0.5,0.5,0.5),vec3(0.0,0.0,1.0),vec2(1,0)),textureID),
                ConstructionVertex(Vertex(vec3(-0.5,-0.5,0.5),vec3(0.0,0.0,1.0),vec2(1,1)),textureID)
            };
            for(auto vertex : faceVerts) {
                vertex.pos = rotation * vertex.pos;
                vertex.pos += position;
                vertex.normal = rotation * vertex.normal;
                vertices.push_back(vertex); 
            }
            auto faceIndices = vector<uint16_t> {
                0,1,2,1,2,3
            };
            for(auto index : faceIndices) {
                indices.push_back(index + indexOffset); 
            }
        }


        bool solidInDirection(ivec3 position,ivec3 direction) {
            Block* block = getBlock(position+direction).first;
            if(block == nullptr) {
                return false;
            }
            if(block->modelType == Block::ModelType::Mesh) {
                return false;
            }
            return true;
        }

        void addSingleBlock(ivec3 position,quat rotation,Block* block) {
            
            if(!solidInDirection(position,ivec3(0,0,1))) addBlockFace(position,rotation,block->texture);
            if(!solidInDirection(position,ivec3(0,0,-1))) addBlockFace(position,rotation * quat(glm::radians(vec3(0,180,0))),block->texture);
            if(!solidInDirection(position,ivec3(1,0,0))) addBlockFace(position,rotation * quat(glm::radians(vec3(0, 90,0))) ,block->texture);
            if(!solidInDirection(position,ivec3(-1,0,0))) addBlockFace(position,rotation * quat(glm::radians(vec3(0,-90,0))) ,block->texture);
            if(!solidInDirection(position,ivec3(0,1,0))) addBlockFace(position,rotation * quat(glm::radians(vec3(-90,0,0))) ,block->texture); //idk why this is flipped from how you'd expect
            if(!solidInDirection(position,ivec3(0,-1,0))) addBlockFace(position,rotation * quat(glm::radians(vec3( 90,0,0))) ,block->texture);
        }

        void addMeshBlock(vec3 position,quat rotation,Block* block) {
            int indexOffset = vertices.size();
            for(auto vertex : block->mesh->meshData.vertices) {
                vertex.pos += position;
                vertices.push_back(ConstructionVertex(vertex,block->texture)); 
            }
            for(auto index : block->mesh->meshData.indices) {
                indices.push_back(index + indexOffset); 
            }
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            if(vertices.size() == 0 || indices.size() == 0) return;
            if(meshState == -1) {
                
                meshBuffer[0] = vulkan->createMeshBuffers(vertices,indices);
                //meshBuffer[1] = vulkan->createMeshBuffers(vertices,indices);
                gpuMeshOutOfDate = false;
                meshState = 0;
            } else {
                if(gpuMeshOutOfDate) {
                    meshState++;
                    if(meshState >= FRAMES_IN_FLIGHT) meshState = 0;
                    vulkan->updateMeshBuffer(meshBuffer[meshState],vertices,indices);
                    gpuMeshOutOfDate = false;
                }
            }
            if(meshState != -1) {
                vulkan->addMesh(meshBuffer[meshState],material,getTransform());
            }
        }

        void setBounds(ivec3 newMin,ivec3 newMax) {
            if(newMin.x > newMax.x || newMin.y > newMax.y || newMin.x > newMax.x) {
                Debug::warn("construction bounds min bigger than max, cancelling operation");
                return;
            }
            
            
            //construct a map of existing blocks
            map<Location,BlockData> blockMap;
            createBlockMap(blockMap);
            min = newMin;
            max = newMax;

            //int i = 0;
            blockData.clear();
            blockCountX.clear();
            blockCountX.resize((newMax.x - newMin.x) + 1);
            for (int z = newMin.z; z <= newMax.z; z++)
            {
                for (int y = newMin.y; y <= newMax.y; y++)
                {
                    for (int x = newMin.x; x <= newMax.x; x++)
                    {
                        auto location = Location(x,y,z);
                        auto it = blockMap.find(location);
                        if(it != blockMap.end()) {
                            //std::cout << "<" << x << "," << y << "," << z <<  ">:" << blockMap.at(Location(x,y,z)) << std::endl;
                            blockData.push_back(it->second);
                            blockCountX[x - newMin.x]++;
                        } else {
                            //std::cout << "e,";
                            blockData.push_back(BlockData());
                        }
                        //i++;
                    }
                }
            }
        }

        virtual std::optional<RaycastHit> raycast(Ray ray, float dist) {
            int i = 0;
            std::optional<RaycastHit> result = std::nullopt;
            Ray localRay = Ray(inverseTransformPoint(ray.origin),inverseTransformDirection(ray.direction));
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        if(blockData[i].block != nullptr) {
                            auto hitopt = Physics::intersectRayBox(vec3(x,y,z),vec3(0.5),localRay);
                            if(hitopt) {
                                
                                auto hit = hitopt.value();
                                if(hit.distance <= dist) {
                                    hit.point = transformPoint(hit.point);
                                    hit.normal = transformDirection(hit.normal);
                                    result = hit;
                                    dist = hit.distance; //distance stays the same when transformed
                                }
                            }
                        }
                        i++;
                    }
                }
            }
            return result;
        }

        virtual void collideBasic(Actor* actor,float height,float radius) {
            vec3 localActorPosition = inverseTransformPoint(actor->position);
            vec3 localActorPositionTop = inverseTransformPoint(actor->transformPoint(vec3(0,height,0)));
            float bounds = radius + height;
            for (int z = floor(std::max((float)min.z,localActorPosition.z-bounds)); z <= ceil(std::min((float)max.z,localActorPosition.z+bounds)); z++)
            {
                for (int y = floor(std::max((float)min.y,localActorPosition.y-bounds)); y <= ceil(std::min((float)max.y,localActorPosition.y+bounds)); y++)
                {
                    for (int x = floor(std::max((float)min.x,localActorPosition.x-bounds)); x <= ceil(std::min((float)max.x,localActorPosition.x+bounds)); x++)
                    {
                        int i = getIndex(ivec3(x,y,z));
                        if(blockData[i].block != nullptr) {
                            auto contact_opt = Physics::intersectCapsuleBox(localActorPosition,localActorPositionTop,radius,vec3(x,y,z),vec3(0.5f));
                            if(contact_opt) {
                                
                                auto contact = contact_opt.value();
                                Physics::resolveBasic(localActorPosition,contact);
                                Physics::resolveBasic(localActorPositionTop,contact);
                                
                            }
                        }
                        Debug::drawCube(transformPoint(vec3(x,y,z)),vec3(1),Color::green);
                    }
                }
            }
            actor->position = transformPoint(localActorPosition);
        }

        void setBlock(ivec3 location,Block* block,BlockFacing facing) {
            if(!isInsideBounds(location)) {
                setBounds(glm::min(min,location),glm::max(max,location));
            }
            int index = getIndex(location);

             
            if(blockData[index].block != nullptr) {
                blockData[index].block->onBreak(this,location,blockData[index].state);
                blockCount--;
                blockCountX[location.x - min.x]--;

                if(blockStorage.contains(Location(location))) {
                    blockStorage.erase(Location(location));
                }
                if(stepCallbacks.contains(Location(location))) {
                    stepCallbacks[Location(location)] = false;
                }
            }
            //Debug::info("index: " + std::to_string(index) + " " + StringHelper::toString(location),InfoPriority::LOW);
            if(block != nullptr) {
                //auto collider = body->addCollider(common->createBoxShape(rp3d::Vector3(0.5f,0.5f,0.5f)),rp3d::Transform(PhysicsHelper::toRp3dVector(location),rp3d::Quaternion::identity()));
                //collider->getMaterial().setBounciness(0.0);
                //collider->getMaterial().setMassDensity(1);
                //body->updateMassPropertiesFromColliders(); //center of mass + inertia tensor included
                blockData[index] = BlockData(block,BlockState(facing));
                if(blockData[index].block != nullptr) {
                    blockData[index].block->onPlace(this,location,blockData[index].state);
                    blockCount++;
                    blockCountX[location.x - min.x]++;
                }
            } else {
                blockData[index] = BlockData();
            }

            generateMesh();
            
            if(blockCount <= 0) {
                destroy();
            }
        }

        void setMoveControl(vec3 move) {
            moveControl = move;
        }

        void setTargetRotation(quat target) {
            targetRotation = target;
        }

        void resetTargets() {
            moveControl = vec3(0,0,0);
            targetRotation = rotation;
        }

        size_t getIndex(ivec3 location) {
            ivec3 fromMin = location - min;
            ivec3 size = max - min;
            size.x += 1;
            size.y += 1;
            size.z += 1;
            int i = std::max(fromMin.x + fromMin.y * size.x + fromMin.z * size.y * size.x,0);
            return std::min((size_t)i,blockData.size()-1); //I think this wont cause issues with out of bounds stuff
        }

        std::pair<Block*,BlockState> getBlock(ivec3 location) {
            if(!isInsideBounds(location)) {
                return std::pair(nullptr,BlockState());
            }
            auto data = blockData[getIndex(location)];
            return std::pair(data.block,data.state);
        }

        vec3 blockCenterLocal(ivec3 location) {
            return vec3(location.x+0.5f,location.y+0.5f,location.z+0.5f);
        }

        bool isInsideBounds(ivec3 location) {
            if(location.x > max.x) return false;
            if(location.y > max.y) return false;
            if(location.z > max.z) return false;

            if(location.x < min.x) return false;
            if(location.y < min.y) return false;
            if(location.z < min.z) return false;
            return true;
        }

        
        BlockStorage* addStorage(ivec3 location) {
            blockStorage[Location(location)] = BlockStorage();
            return &blockStorage[Location(location)];
        }

        void addStepCallback(ivec3 location) {
            stepCallbacks[Location(location)] = true;
        }

        BlockStorage* getStorage(ivec3 location) {
            if(blockStorage.contains(Location(location))) {
                return &blockStorage[Location(location)];
            }
            return nullptr;
        }

        void createBlockMap(map<Location,BlockData>& blockMap) {
            int i = 0;
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        if(blockData[i].block != nullptr) {
                            blockMap[Location(x,y,z)] = blockData[i];
                        }
                        i++;
                    }
                }
            }
        }

        static std::unique_ptr<Construction> makeInstance(Material material,vec3 position,quat rotation = glm::identity<quat>()) {
            auto ptr = new Construction();
            ptr->position = position;
            ptr->rotation = rotation;
            ptr->targetRotation = rotation;
            ptr->material = material;
            return std::unique_ptr<Construction>(ptr);
        }

        static std::unique_ptr<Construction> makeInstance(Material material,Block* block,vec3 position,quat rotation = glm::identity<quat>()) {
            auto ptr = makeInstance(material,position,rotation);
            ptr->setBlock(ivec3(0),block,BlockFacing::FORWARD);
            return ptr;
        }
};

bool operator< (const Construction::Location& a,const Construction::Location& b) {
    if(a.x == b.x) {
        if(a.y == b.y) {
            return a.z < b.z;
        }
        return a.y < b.y;
    }
    return a.x < b.x;
}