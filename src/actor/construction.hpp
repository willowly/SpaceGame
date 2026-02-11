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

#include "helper/rect.hpp"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include <physics/jolt-conversions.hpp>
#include <physics/jolt-layers.hpp>
#include <physics/jolt-userdata.hpp>

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

    static const Color debugGroupColors[];

    struct BlockData {
        Block* block = nullptr; //eventually this will be a list of blocks in the construction
        BlockState state = BlockState::none;
        std::optional<JPH::Shape*> shapeOpt;
        BlockData(Block* block,BlockState state) : block(block), state(state) {}
        BlockData() {}
    };

    ivec3 min = {};
    ivec3 max = {};
    std::vector<BlockData> blockData;
    MeshData<ConstructionVertex> meshData;

    Material material = Material::none;

    int meshState = -1;
    MeshBuffer meshBuffer[FRAMES_IN_FLIGHT];
    bool gpuMeshOutOfDate = false;
    //bool meshOutOfDate;

    std::vector<int> blockCountX;
    std::vector<int> blockCountY;
    std::vector<int> blockCountZ;

    vec3 moveControl = {};
    quat targetRotation = glm::identity<quat>();
    bool targetRotationEnabled = false;
    float turnForce = 1.5;
    vec3 velocity = {};

    int blockCount = 0; //block count

    float timer = 0;

    bool isStatic = false;

    bool physicsShapeChanged = false;

    vec3 accumulatedThrustForce; // acculated on step, applied on prePhysics

    JPH::Body *body;
    JPH::MutableCompoundShape* physicsShape;
    JPH::Vec3 physicsOldCOM;

    Construction() : Actor() {
        blockData.push_back(BlockData()); //starting block
        blockCountX.push_back(0);
        blockCountY.push_back(0);
        blockCountZ.push_back(0);
        
        
        JPH::MutableCompoundShapeSettings shapeSettings{};
        JPH::Shape::ShapeResult result;

        physicsShape = new JPH::MutableCompoundShape(shapeSettings,result);
        physicsShape->SetEmbedded();

        if(result.HasError()) {
            std::cout << result.GetError() << std::endl;
        }
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
                    BlockState blockState = BlockState::none;
                    std::tie(block,blockState) = getBlock(location);

                    if(block != nullptr) {
                        block->onStep(world,this,location,blockState,dt);
                    }
                }
            }

            timer += dt;

            accumulatedThrustForce = vec3(0.0f);
            //i have no idea why z and x are inverted :shrug:
            if(moveControl.z > 0.01) {
                applyForce(vec3(0,0,1) * thrustForces[BlockFacing::BACKWARD] * moveControl.z); //too move forward we must thrust backwards
            }
            if(moveControl.z < 0.01) {
                applyForce(vec3(0,0,1) * thrustForces[BlockFacing::FORWARD] * moveControl.z);
            }
            if(moveControl.x < 0.01) {
                applyForce(vec3(1,0,0) * thrustForces[BlockFacing::RIGHT] * moveControl.x);
            }
            if(moveControl.x > 0.01) {
                applyForce(vec3(1,0,0) * thrustForces[BlockFacing::LEFT] * moveControl.x);
            }
            if(moveControl.y < 0.01) {
                applyForce(vec3(0,1,0) * thrustForces[BlockFacing::UP] * moveControl.y);
            }
            if(moveControl.y > 0.01) {
                applyForce(vec3(0,1,0) * thrustForces[BlockFacing::DOWN] * moveControl.y);
            }
            
            if(targetRotationEnabled && !isStatic) {
                
                rotation = glm::slerp(rotation,targetRotation,turnForce * dt);
                body->SetAngularVelocity(JPH::Vec3(0,0,0)); //maybe we have an actual variable for this instead of just relying
            }
            
        }

        // maybe we should accululate force and apply it prephysics :shrug:
        void applyForce(vec3 force) {
            accumulatedThrustForce += transformDirection(force);
        }

        void generateMesh() {

            //if(!meshOutOfDate) return;

            meshData.clear();

            size_t i = 0;
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        
                        if(blockData.size() > i && blockData[i].block != nullptr) {
                            
                            auto block = blockData[i].block;
                            auto state = blockData[i].state;
                            block->addToMesh(this,meshData,ivec3(x,y,z),state);
                            
                        }
                        i++;
                    }
                }
            }

            gpuMeshOutOfDate = true;


        }

        bool solidInDirection(ivec3 position,ivec3 direction) {
            Block* block = getBlock(position+direction).first;
            if(block == nullptr) {
                return false;
            }
            // check if block is "solid"
            if(!block->solid) {
                return false;
            }
            
            return true;
        }

        // end move a bunch of these to helper

        void addRenderables(Vulkan* vulkan,float dt) {
            if(meshData.vertices.size() == 0 || meshData.indices.size() == 0) return;
            if(meshState == -1) {
                
                meshBuffer[0] = vulkan->createMeshBuffers(meshData.vertices,meshData.indices);
                //meshBuffer[1] = vulkan->createMeshBuffers(vertices,indices);
                gpuMeshOutOfDate = false;
                meshState = 0;
            } else {
                if(gpuMeshOutOfDate) {
                    meshState++;
                    if(meshState >= FRAMES_IN_FLIGHT) meshState = 0;
                    vulkan->updateMeshBuffer(meshBuffer[meshState],meshData.vertices,meshData.indices);
                    gpuMeshOutOfDate = false;
                }
            }
            if(meshState != -1) {
                vulkan->addMesh(meshBuffer[meshState],material,getTransform());
            }
            // center of mass
            // Debug::drawPoint(position,Color::green);

            // thrust forces
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::FORWARD]* vec3(0,0,0.1)), Color::blue);
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::BACKWARD]*vec3(0,0,-0.1)),Color::blue);
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::UP]*      vec3(0,0.1,0)),Color::yellow);
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::DOWN]*    vec3(0,-0.1,0)), Color::yellow);
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::RIGHT]*   vec3(0.1,0,0)), Color::red);
            Debug::drawRay(position,transformDirection(thrustForces[BlockFacing::LEFT]*    vec3(-0.1,0,0)),Color::red);

            // bounds
            Debug::drawCube(transformPoint((vec3)(max+min)/2.0f),(vec3)(max-min) + vec3(1),rotation,Color::red);

            for (auto subShape : physicsShape->GetSubShapes())
            {
                Debug::drawPoint(transformPoint(Physics::toGlmVec(subShape.GetPositionCOM())));
            }
            
        }

        void moveBounds(ivec3 amount) {
            min += amount;
            max += amount;
        }

        void setBounds(ivec3 newMin,ivec3 newMax) {
            if(newMin.x > newMax.x || newMin.y > newMax.y || newMin.x > newMax.x) {
                Debug::warn("construction bounds MIN bigger than MAX, cancelling operation");
                return;
            }
            
            //construct a map of existing blocks
            map<Location,BlockData> blockMap;
            createBlockMap(blockMap);
            min = newMin;
            max = newMax;

            //int i = 0;
            blockData.clear();
            blockCountX.clear(); // this kinda repeated code could probably be written in a more elegant way but Idk how so :shrug:
            blockCountX.resize((newMax.x - newMin.x) + 1);
            blockCountY.clear();
            blockCountY.resize((newMax.y - newMin.y) + 1);
            blockCountZ.clear();
            blockCountZ.resize((newMax.z - newMin.z) + 1);
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
                            blockCountY[y - newMin.y]++;
                            blockCountZ[z - newMin.z]++;
                        } else {
                            //std::cout << "e,";
                            blockData.push_back(BlockData());
                        }
                        //i++;
                    }
                }
            }

            //printBlockCounts();

            
        }

        virtual void spawn(World* world) {
            
            addBody(world);


        }

        void addBody(World* world) {
            JPH::BodyCreationSettings bodySettings;

            if(isStatic) {
                bodySettings = JPH::BodyCreationSettings(physicsShape, Physics::toJoltVec(position), Physics::toJoltQuat(position), JPH::EMotionType::Static, Layers::NON_MOVING);
            } else {
                bodySettings = JPH::BodyCreationSettings(physicsShape, Physics::toJoltVec(position), Physics::toJoltQuat(position), JPH::EMotionType::Dynamic, Layers::MOVING);
            }

            bodySettings.mGravityFactor = 0.0f;

            bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            bodySettings.mMassPropertiesOverride.mMass = 1.0f;
            // bodySettings.mUserData = ActorUserData(this).asUInt();
            body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
            world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);
            std::cout << "new construction created with ID " << body->GetID().GetIndexAndSequenceNumber() << std::endl;
            

            // otherwise will error :3
            if(!isStatic) body->SetLinearVelocity(Physics::toJoltVec(velocity));
            body->SetRestitution(0.1f);
            body->SetFriction(2.0);

            
        }
        

        virtual void prePhysics(World* world) {
            if(physicsShapeChanged) {
                physicsShapeChanged = false;
                world->physics_system.GetBodyInterface().NotifyShapeChanged(body->GetID(),physicsOldCOM,false,JPH::EActivation::Activate);
            }
            if(!isStatic) {
                if(physicsShape->GetMassProperties().mMass > 0.0f) {
                    body->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::All,physicsShape->GetMassProperties());
                }
            }
            
            world->physics_system.GetBodyInterface().SetPosition(body->GetID(),Physics::toJoltVec(position),JPH::EActivation::DontActivate);
            world->physics_system.GetBodyInterface().SetRotation(body->GetID(),Physics::toJoltQuat(rotation),JPH::EActivation::DontActivate);
            if(!isStatic) world->physics_system.GetBodyInterface().SetLinearVelocity(body->GetID(),Physics::toJoltVec(velocity)); //if static it errors :3
            world->physics_system.GetBodyInterface().AddForce(body->GetID(),Physics::toJoltVec(accumulatedThrustForce));
        }

        virtual void postPhysics(World* world) {
            position = Physics::toGlmVec(body->GetPosition());
            rotation = Physics::toGlmQuat(body->GetRotation().Normalized());
            velocity = Physics::toGlmVec(body->GetLinearVelocity());
        }

        void destroy(World* world) {
            Actor::destroy(world);

            world->physics_system.GetBodyInterface().RemoveBody(body->GetID());
            world->physics_system.GetBodyInterface().DestroyBody(body->GetID());
        }

        void printBlockCounts() {

            std::cout << "block x: ";
            for (size_t i = 0; i < blockCountX.size(); i++)
            {
                std::cout << blockCountX[i] << ",";
            }
            std::cout << std::endl;
            std::cout << "block y: ";
            for (size_t i = 0; i < blockCountY.size(); i++)
            {
                std::cout << blockCountY[i] << ",";
            }
            std::cout << std::endl;
            std::cout << "block z: ";
            for (size_t i = 0; i < blockCountZ.size(); i++)
            {
                std::cout << blockCountZ[i] << ",";
            }
            std::cout << std::endl;
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
            vec3 localActorPosition = inverseTransformPoint(actor->getPosition());
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
                    }
                }
            }
            actor->setPosition(transformPoint(localActorPosition));
        }

        vec3 getVelocity() {
            return velocity;
        }

        // END RIGIDBODY COMPONENT

        // used when wanting to break a place, not replace
        void breakBlock(World* world,ivec3 location) {

            removeBlockNoUpdate(location); //does the main removal, tracking, and storage/callback cleanup 

            recalculateBoundsFromBreak(location); //reduce bounds or break up construction. makes index invalid

            //recalculatePivot();
            generateMesh();

            calculateBreakup(world,location);

            if(blockCount <= 0) {
                destroy(world);
            }
        }


        // used by placing and breaking. Handles removal, tracking, physics, and storage/callback cleanup
        void removeBlockNoUpdate(ivec3 location) {

            int index = getIndex(location);

            if(blockData[index].block != nullptr) {
                blockData[index].block->onBreak(this,location,blockData[index].state);
                blockCount--;
                blockCountX[location.x - min.x]--;
                blockCountY[location.y - min.y]--;
                blockCountZ[location.z - min.z]--;
                
                if(blockData[index].shapeOpt) {
                    
                    //JPH::SubShapeID remainder;
                    //JPH::uint shapeIndex = physicsShape->GetSubShapeIndexFromID(blockData[index].shapeOpt.value().GetID(),remainder);

                    for (size_t i = 0; i < physicsShape->GetSubShapes().size(); i++)
                    {
                        if(blockData[index].shapeOpt.value() == physicsShape->GetSubShape(i).mShape.GetPtr()) {
                            physicsShape->RemoveShape(i);
                            physicsShape->AdjustCenterOfMass();
                            physicsShapeChanged = true;
                        }
                    }
                }

                blockData[index] = BlockData();

                


                if(blockStorage.contains(Location(location))) {
                    blockStorage.erase(Location(location));
                }
                if(stepCallbacks.contains(Location(location))) {
                    stepCallbacks[Location(location)] = false;
                }
            }
        }

        //assumes its not replacing a block. handles tracking, mass and bounds
        void addBlockNoUpdate(ivec3 location,Block& block,BlockFacing facing) {
            blockCount++;
            blockCountX[location.x - min.x]++;
            blockCountY[location.y - min.y]++;
            blockCountZ[location.z - min.z]++;
            int index = getIndex(location);
            auto state = block.onPlace(this,location,facing);
            blockData[index] = BlockData(&block,state);

            physicsOldCOM = physicsShape->GetCenterOfMass(); //temporary until custom shape (hopefully)
            auto shape = new JPH::BoxShape(JPH::Vec3(0.5f,0.5f,0.5f));
            physicsShape->AddShape(Physics::toJoltVec((vec3)location),JPH::Quat::sIdentity(),shape,0,UINT_MAX);
            blockData[index].shapeOpt = static_cast<JPH::Shape*>(shape);
            physicsShape->AdjustCenterOfMass();
            physicsShapeChanged = true;

            
        }

        // make the bounds
        void boundsEncapsulate(ivec3 location) {
            if(!isInsideBounds(location)) {
                vec3 newMin = glm::min(min,location);
                vec3 newMax = glm::max(max,location);
                setBounds(newMin,newMax);
            }
        }

        // places a block, must be valid. can't be used for destruction. replaces existing block. Handles updating mesh and stuff
        void placeBlock(ivec3 location,Block* block,BlockFacing facing) {
            if(block == nullptr) {
                Debug::warn("tried to place null block");
                return;
            }

            boundsEncapsulate(location);
            
            removeBlockNoUpdate(location);
            
            
            addBlockNoUpdate(location,*block,facing);

            //recalculatePivot();
            generateMesh();
        }

        void calculateBreakup(World* world,ivec3 location) {
            std::vector<int> blockGroups(blockData.size(),0);
            std::vector<ivec3> locations;
            std::cout << "STARTING BREAKUP CALCULATION" << std::endl;
            tryAddToGroup(1,location+ivec3(1,0,0),blockGroups,locations);
            tryAddToGroup(2,location+ivec3(-1,0,0),blockGroups,locations);
            tryAddToGroup(3,location+ivec3(0,1,0),blockGroups,locations);
            tryAddToGroup(4,location+ivec3(0,-1,0),blockGroups,locations);
            tryAddToGroup(5,location+ivec3(0,0,1),blockGroups,locations);
            tryAddToGroup(6,location+ivec3(0,0,-1),blockGroups,locations);

            const ivec3 directions[] = {
                ivec3(1,0,0),
                ivec3(-1,0,0),
                ivec3(0,1,0),
                ivec3(0,-1,0),
                ivec3(0,0,1),
                ivec3(0,0,-1)
            };

            std::vector<Construction*> constructions(6,nullptr);

            int originalConstructionGroup = 0;
            int i = -1; //increments at the start
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        i++; // so continues work
                        int blockGroup = blockGroups[i];
                        if(blockGroup != 0) {

                            if(blockGroup == originalConstructionGroup) continue;

                            vec3 worldLocation = transformPoint(vec3(x,y,z));
                            if(constructions[blockGroup-1] == nullptr) {
                                if(originalConstructionGroup == 0) {
                                    originalConstructionGroup = blockGroup;
                                    constructions[blockGroup-1] = this;
                                    continue;
                                } else {
                                    constructions[blockGroup-1] = world->spawn(Construction::makeInstance(material,worldLocation,rotation,isStatic));
                                    constructions[blockGroup-1]->velocity = velocity; //match velocities
                                }
                            }
                            auto& newConstruction = *constructions[blockGroup-1];
                            ivec3 newLocalLocation = glm::round(newConstruction.inverseTransformPoint(worldLocation));
                            int newIndex = newConstruction.getIndex(newLocalLocation);
                            if(blockGroup != originalConstructionGroup) {
                                newConstruction.boundsEncapsulate(newLocalLocation);
                                newConstruction.addBlockNoUpdate(newLocalLocation,*blockData[i].block,BlockFacing::FORWARD); //a block location should not have a group unless its a real block
                                newConstruction.blockData[newIndex].state = blockData[i].state; // copy state
                                // TODO: transfer storage
                                removeBlockNoUpdate(vec3(x,y,z));
                            }

                        }
                    }
                }
            }

            for (size_t i = 0; i < 6; i++)
            {
                if(constructions[i] != nullptr) { //regenerate all at the end
                    constructions[i]->generateMesh();
                    constructions[i]->recalculateBounds();
                }
            }
            

        }

        void tryAddToGroup(int group,ivec3 location,std::vector<int>& blockGroups,std::vector<ivec3>& locations) {
            if(getBlock(location).first == nullptr) {
                //std::cout << "block is null" << std::endl;
                return;
            }
            int index = getIndex(location);
            if(blockGroups[index] >= group) {
                //std::cout << "group is " << group << std::endl;
                return;
            }
            Debug::drawCube(transformPoint(location),vec3(1),rotation,debugGroupColors[group-1],1);
            blockGroups[index] = group;
            //locations.push_back(location);
            tryAddToGroup(group,location+ivec3(1,0,0),blockGroups,locations);
            tryAddToGroup(group,location+ivec3(-1,0,0),blockGroups,locations);
            tryAddToGroup(group,location+ivec3(0,1,0),blockGroups,locations);
            tryAddToGroup(group,location+ivec3(0,-1,0),blockGroups,locations);
            tryAddToGroup(group,location+ivec3(0,0,1),blockGroups,locations);
            tryAddToGroup(group,location+ivec3(0,0,-1),blockGroups,locations);
        }

        // recalculate bounds using the block counts
        void recalculateBounds() {
            ivec3 newMin = ivec3(0);
            ivec3 newMax = ivec3(0);

            printBlockCounts();

            bool minSet = false;
            for (size_t i = 0; i < blockCountX.size(); i++)
            {
                if(blockCountX[i] != 0) {
                    newMax.x = i + min.x;
                    if(!minSet) {
                        minSet = true;
                        newMin.x = i + min.x;
                    }
                }
            }
            minSet = false;
            for (size_t i = 0; i < blockCountY.size(); i++)
            {
                if(blockCountY[i] != 0) {
                    newMax.y = i + min.y;
                    if(!minSet) {
                        minSet = true;
                        newMin.y = i + min.y;
                    }
                }
            }
            minSet = false;
            for (size_t i = 0; i < blockCountZ.size(); i++)
            {
                if(blockCountZ[i] != 0) {
                    newMax.z = i + min.z;
                    if(!minSet) {
                        minSet = true;
                        newMin.z = i + min.z;
                    }
                }
            }
            if(newMin != min || newMax != max) {
                setBounds(newMin,newMax);
            }
            
        }

        // reduces bounds, call when block broken
        void recalculateBoundsFromBreak(ivec3 location) {

            // TODO should probably accumulate bounds change but its probably literally not an issue at all. It would have to be detached anyway
            ivec3 newMin = min;
            ivec3 newMax = max;
            ivec3 indices = location - min;
            if(blockCountX[indices.x] == 0) {
                if(indices.x == 0) {
                    newMin.x += 1;
                }
                else if(indices.x == blockCountX.size()-1) {
                    newMax.x -= 1;
                }
            }
            if(blockCountY[indices.y] == 0) {
                if(indices.y == 0) {
                    newMin.y += 1;
                }
                else if(indices.y == blockCountY.size()-1) {
                    newMax.y -= 1;
                }
            }
            if(blockCountZ[indices.z] == 0) {
                if(indices.z == 0) {
                    newMin.z += 1;
                }
                else if(indices.z == blockCountZ.size()-1) {
                    newMax.z -= 1;
                }
            }
            if(newMin != min || newMax != max) {
                setBounds(newMin,newMax);
            }
        }

        void setMoveControl(vec3 move) {
            moveControl = move;
        }

        void setTargetRotation(quat target) {
            targetRotation = target;
            targetRotationEnabled = true;
        }

        void resetTargets() {
            moveControl = vec3(0,0,0);
            targetRotation = rotation;
            targetRotationEnabled = false;
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
                return std::pair(nullptr,BlockState::none);
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

        static std::unique_ptr<Construction> makeInstance(Material material,vec3 position,quat rotation = glm::identity<quat>(),bool isStatic = false) {
            auto ptr = new Construction();
            ptr->position = position;
            ptr->rotation = rotation;
            ptr->targetRotation = rotation;
            ptr->material = material;
            ptr->isStatic = isStatic;
            return std::unique_ptr<Construction>(ptr);
        }

        static std::unique_ptr<Construction> makeInstance(Material material,Block* block,vec3 position,quat rotation = glm::identity<quat>(),bool isStatic = false) {
            auto ptr = makeInstance(material,position,rotation,isStatic);
            ptr->placeBlock(ivec3(0),block,BlockFacing::FORWARD);
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

const Color Construction::debugGroupColors[] = {
    Color::green,
    Color::blue,
    Color::red,
    Color::magenta,
    Color::cyan,
    Color::yellow
};