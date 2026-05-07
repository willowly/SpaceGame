#pragma once

#include "actor.hpp"
#include <glm/glm.hpp>
#include <unordered_map>

#include "engine/debug.hpp"
#include "item-actor.hpp"

#include <helper/string-helper.hpp>
#include <algorithm>

#include <block/block.hpp>

#include "helper/block-storage.hpp"

#include "helper/rect.hpp"

#include "physics/jolt-layers.hpp"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

#include <physics/jolt-conversions.hpp>
#include <physics/jolt-layers.hpp>
#include <physics/jolt-userdata.hpp>

#include "persistance/actor/data-construction.hpp"
#include "persistance/data-loader.hpp"

#include "block/block-id.hpp"

using glm::ivec3,glm::vec3;
using std::unordered_map;


struct ConstructionVertex {
    glm::vec3 pos = {};
    glm::vec3 normal = {};
    glm::vec2 texCoord = {};
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


class Construction : public Actor {

    static const Color debugGroupColors[];

    struct BlockPaletteEntry {
        Block* block = nullptr;
        BlockStorage storage;

        bool operator==(const BlockPaletteEntry& entry) {
            if(block != entry.block) return false;
            if(storage != entry.storage) return false;
            return true;
        }

        data_BlockPaletteEntry save() {
            data_BlockPaletteEntry data;
            if(block != nullptr) {
                data.block = block->name;
            }
            data.storage = storage.save();
            return data;
        }

        void load(data_BlockPaletteEntry data,DataLoader& loader) {
            block = loader.getBlockPrototype((string)data.block);
            storage.load(data.storage,loader);
        }
    };

    // we can convert this to just block ID if we use a custom physics shape :shrug: <- not true!!!
    struct BlockData {
        BlockID id = 0;
        bool attached = false;
        std::optional<JPH::Shape*> shapeOpt;
        BlockData(BlockID id,bool attached) : id(id), attached(attached) {}
        BlockData(BlockID id) : id(id) {}
        BlockData() {}

        data_BlockData save() {
            data_BlockData data;
            data.id = id;
            return data;
        }

        void load(data_BlockData data) {
            id = data.id;
        }
    };

    ivec3 boundsMin = {};
    ivec3 boundsMax = {};
    std::vector<BlockPaletteEntry> blockPalette;
    std::vector<BlockData> blockDataArray;
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
    vec3 turnControl = {};
    float turnForce = 1.5;

    int blockCount = 0; //block count

    float timer = 0;

    float turnTorque = 30.0f;

    bool isStatic = false;

    bool physicsShapeChanged = false;

    vec3 accumulatedThrustForce = {}; // acculated on step, applied on prePhysics
    vec3 accumulatedTorque = {};

    // Rigidbody Class
    Rigidbody body;
    JPH::MutableCompoundShape* physicsShape = nullptr;
    JPH::Vec3 physicsOldCOM;
    ActorUserData physicsUserData;

    Construction() : Actor() {
        blockDataArray.push_back(BlockData()); //starting block
        blockCountX.push_back(0);
        blockCountY.push_back(0);
        blockCountZ.push_back(0);
        
        for (size_t i = 0; i < 6; i++)
        {
            thrustForces[i] = 0;
        }

        blockPalette.push_back(BlockPaletteEntry());
        
        Physics::initalizePhysicsGlobal();

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
        std::map<Location,bool> stepCallbacks;

        void step(World* world,float dt) override {

            updateLastTransform();
            
            for(auto& pair : stepCallbacks) {

                ivec3 location = pair.first.asVec3();
                bool enabled = pair.second;
                if(enabled) {
                    auto& entry = getBlock(location);
                    auto& block = entry.block;

                    if(entry.block != nullptr) {
                        entry.block->onStep(world,this,location,entry.storage,dt);
                    }
                }
            }

            timer += dt;

            body.applyGravity(world,position,dt);

            accumulatedThrustForce = vec3(0.0f);
            accumulatedTorque = vec3(0.0f);
            //i have no idea why z and x are inverted :shrug:
            if(moveControl.z > 0.01) {
                applyForce(vec3(0,0,1) * thrustForces[static_cast<int>(BlockFacing::BACKWARD)] * moveControl.z); //too move forward we must thrust backwards
            }
            if(moveControl.z < 0.01) {
                applyForce(vec3(0,0,1) * thrustForces[static_cast<int>(BlockFacing::FORWARD)] * moveControl.z);
            }
            if(moveControl.x < 0.01) {
                applyForce(vec3(1,0,0) * thrustForces[static_cast<int>(BlockFacing::RIGHT)] * moveControl.x);
            }
            if(moveControl.x > 0.01) {
                applyForce(vec3(1,0,0) * thrustForces[static_cast<int>(BlockFacing::LEFT)] * moveControl.x);
            }
            if(moveControl.y < 0.01) {
                applyForce(vec3(0,1,0) * thrustForces[static_cast<int>(BlockFacing::UP)] * moveControl.y);
            }
            if(moveControl.y > 0.01) {
                applyForce(vec3(0,1,0) * thrustForces[static_cast<int>(BlockFacing::DOWN)] * moveControl.y);
            }

            if(turnControl.x < 0.01) {
                applyTorque(transformDirection(vec3(1,0,0)) * turnControl.x * turnTorque);
            }
            if(turnControl.x > 0.01) {
                applyTorque(transformDirection(vec3(1,0,0)) * turnControl.x * turnTorque);
            }
            if(turnControl.y < 0.01) {
                applyTorque(transformDirection(vec3(0,1,0)) * turnControl.y * turnTorque);
            }
            if(turnControl.y > 0.01) {
                applyTorque(transformDirection(vec3(0,1,0)) * turnControl.y * turnTorque);
            }
            if(turnControl.z < 0.01) {
                applyTorque(transformDirection(vec3(0,0,1)) * turnControl.z * turnTorque);
            }
            if(turnControl.z > 0.01) {
                applyTorque(transformDirection(vec3(0,0,1)) * turnControl.z * turnTorque);
            }

            vec3 angularVelocity = body.getAngularVelocity();
            angularVelocity += accumulatedTorque * dt;
            body.setAngularVelocity(angularVelocity);
            
            
        }

        // maybe we should accululate force and apply it prephysics :shrug:
        void applyForce(vec3 force) {
            accumulatedThrustForce += transformDirection(force);
        }

        void applyTorque(vec3 torque) {
            accumulatedTorque += torque;
        }

        void generateMesh() {

            //if(!meshOutOfDate) return;

            meshData.clear();

            size_t i = 0;
            for (int z = boundsMin.z; z <= boundsMax.z; z++)
            {
                for (int y = boundsMin.y; y <= boundsMax.y; y++)
                {
                    for (int x = boundsMin.x; x <= boundsMax.x; x++)
                    {
                        
                        if(blockDataArray.size() <= i) break;

                        auto data = blockDataArray[i];
                        
                        if(data.id != 0) { //optimization i guess
                            auto entry = blockPalette[data.id];
                            auto block = entry.block;
                            block->addToMesh(this,meshData,ivec3(x,y,z),entry.storage);
                        }
                        
                        i++;
                    }
                }
            }

            gpuMeshOutOfDate = true;


        }

        bool solidInDirection(ivec3 position,ivec3 direction) {
            Block* block = getBlock(position+direction).block;
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

        void addRenderables(Vulkan* vulkan,float dt,float interpolation) override {
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
                vulkan->addMesh(meshBuffer[meshState],material,getInterpolatedTransform(interpolation));
            }

            Debug::drawRay(position,transformDirection(vec3(0,0,1)),Color::green);
            Debug::drawRay(position,body.getAngularVelocity(),Color::blue);
            Debug::drawRay(position,turnControl,Color::red);
            
        }

        // <min,max>
        std::pair<ivec3,ivec3> getBounds() {
            return std::pair(boundsMin,boundsMax);
        }

        void moveBounds(ivec3 amount) {
            boundsMin += amount;
            boundsMax += amount;
        }

        void setBounds(ivec3 newMin,ivec3 newMax) {
            if(newMin.x > newMax.x || newMin.y > newMax.y || newMin.x > newMax.x) {
                Debug::warn("construction bounds MIN bigger than MAX, cancelling operation");
                return;
            }
            
            //construct a map of existing blocks
            map<Location,BlockData> blockMap;
            createBlockMap(blockMap);
            boundsMin = newMin;
            boundsMax = newMax;

            //int i = 0;
            blockDataArray.clear();
            blockCount = 0; //because addBlockCount adds the count
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
                            blockDataArray.push_back(it->second);
                            addBlockCount(ivec3(x,y,z));
                        } else {
                            //std::cout << "e,";
                            blockDataArray.push_back(BlockData());
                        }
                        //i++;
                    }
                }
            }

            //printBlockCounts();

            
        }

        void setIsStatic(bool value) {
            if(value != isStatic) {
                isStatic = value;
            }
        }

        JPH::BodyID getBodyID() {
            return body.getBodyID();
        }

        void spawn(World* world) override {
            
            spawnBody(world);

        }
        
        void spawnBody(World* world) {
            auto bodySettings = body.getDefaultBodySettings(this,physicsShape,position,rotation);

            if(isStatic) {
                bodySettings.mMotionType = JPH::EMotionType::Static;
                bodySettings.mObjectLayer = Layers::NON_MOVING;
                body.setVelocity(vec3(0));
            }

            bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
            bodySettings.mMassPropertiesOverride.mMass = 1.0f;
            bodySettings.mAllowDynamicOrKinematic = true;

            body.spawn(world,this,bodySettings);
            body.getBody()->SetRestitution(0.1f);
            body.getBody()->SetFriction(2.0);
        }

        // get the maximum angular acc for rotation v
        float getMaxAngularAccelerationYaw() {
            return turnTorque;
            //return body.getBody()->GetMotionProperties()->MultiplyWorldSpaceInverseInertiaByVector(Physics::toJoltQuat(rotation),Physics::toJoltVec(transformDirection(vec3(0,100.0f,0)))).GetY();
        }

        float getMaxAngularAccelerationPitch() {
            return turnTorque;
            //return body.getBody()->GetMotionProperties()->MultiplyWorldSpaceInverseInertiaByVector(Physics::toJoltQuat(rotation),Physics::toJoltVec(transformDirection(vec3(0,100.0f,0)))).GetY();
        }

        float getMaxAngularAccelerationRoll() {
            return turnTorque;
            //return body.getBody()->GetMotionProperties()->MultiplyWorldSpaceInverseInertiaByVector(Physics::toJoltQuat(rotation),Physics::toJoltVec(transformDirection(vec3(0,100.0f,0)))).GetY();
        }
        

        virtual void prePhysics(World* world) {
            if(physicsShapeChanged) {
                physicsShapeChanged = false;
                world->physics_system.GetBodyInterface().NotifyShapeChanged(body.getBodyID(),physicsOldCOM,false,JPH::EActivation::Activate);
            }
            if(!isStatic) {
                body.getBody()->SetMotionType(JPH::EMotionType::Dynamic);
                world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::MOVING);
                if(physicsShape->GetMassProperties().mMass > 0.0f) {
                    body.getBody()->GetMotionProperties()->SetMassProperties(JPH::EAllowedDOFs::All,physicsShape->GetMassProperties());
                }
            } else {
                if(body.getBody()->GetMotionType() != JPH::EMotionType::Static) {
                    world->physics_system.GetBodyInterface().DeactivateBody(body.getBodyID());
                }
                body.getBody()->SetMotionType(JPH::EMotionType::Static);
                world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::NON_MOVING);
            }

            auto bodyID = body.getBodyID();
            if(!isStatic) {
                body.prePhysics(world,position,rotation);
                world->physics_system.GetBodyInterface().AddForce(bodyID,Physics::toJoltVec(accumulatedThrustForce));                                                                                                                                                                                                                                                                                         
            } else {
                world->physics_system.GetBodyInterface().SetPosition(bodyID,Physics::toJoltVec(position),JPH::EActivation::DontActivate);
                world->physics_system.GetBodyInterface().SetRotation(bodyID,Physics::toJoltQuat(rotation),JPH::EActivation::DontActivate);
            }
            
            
        }

        virtual void postPhysics(World* world) {
            body.postPhysics(world,position,rotation);
            if(isStatic) {
                body.setVelocity(vec3(0)); // so we dont get invalid velocity
            }
        }

        void destroy(World* world) {
            Actor::destroy(world);

            body.destroy(world);
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

        vec3 getVelocity() {
            return body.getVelocity();
        }

        vec3 getAngularVelocity() {
            return body.getAngularVelocity();
        }

        // END RIGIDBODY COMPONENT

        // used when wanting to break a place, not replace
        void breakBlock(World* world,ivec3 location) {

            auto& blockEntry = getBlock(location);
            auto block = blockEntry.block;
            assert(block != nullptr);
            auto items = block->getDrops(this,location,blockEntry.storage);

            for(auto stack : items) {
                world->spawn(ItemActor::makeInstance(stack,transformPoint(location)));
            }

            removeBlockNoUpdate(location); //does the main removal, tracking, and storage/callback cleanup 

            recalculateBoundsFromBreak(location); //reduce bounds or break up construction. makes index invalid

            //recalculatePivot();
            generateMesh();

            calculateBreakup(world,location);

            //printIDList();

            if(blockCount <= 0) {
                destroy(world);
            }
        }


        // used by placing and breaking. Handles removal, tracking, physics, and storage/callback cleanup
        void removeBlockNoUpdate(ivec3 location) {

            int index = getIndex(location);

            auto& blockData = blockDataArray.at(index);
            auto& blockEntry = blockPalette.at(blockData.id);

            if(blockEntry.block != nullptr) {
                blockEntry.block->onBreak(this,location,blockEntry.storage);
                removeBlockCount(location);
                
                if(blockData.shapeOpt) {
                    
                    //JPH::SubShapeID remainder;
                    //JPH::uint shapeIndex = physicsShape->GetSubShapeIndexFromID(blockData[index].shapeOpt.value().GetID(),remainder);

                    for (size_t i = 0; i < physicsShape->GetSubShapes().size(); i++)
                    {
                        if(blockData.shapeOpt.value() == physicsShape->GetSubShape(i).mShape.GetPtr()) {
                            physicsShape->RemoveShape(i);
                            physicsShape->AdjustCenterOfMass();
                            physicsShapeChanged = true;
                        }
                    }
                }

                blockData = BlockData(0);

                
                if(stepCallbacks.contains(Location(location))) {
                    stepCallbacks[Location(location)] = false;
                }
            }
        }


        BlockID paletteEntryToID(BlockPaletteEntry entry) {
            if(entry.block != nullptr && entry.block->getStorageType() == Block::StorageType::Constant) {
                for (size_t i = 0; i < blockPalette.size(); i++)
                {
                    if(blockPalette.at(i).operator==(entry)) {
                        return i;
                    }
                }
            }
            
            return newPaletteEntry(std::move(entry));
        }

        BlockID newPaletteEntry(BlockPaletteEntry entry) {
            blockPalette.push_back(std::move(entry));
            return blockPalette.size() - 1;
        }
        
        void printIDList() {

            int i = 0;
            for (int z = boundsMin.z; z <= boundsMax.z; z++)
            {
                for (int y = boundsMin.y; y <= boundsMax.y; y++)
                {
                    for (int x = boundsMin.x; x <= boundsMax.x; x++)
                    {
                        std::cout << blockDataArray.at(i).id << " ";
                        i++;
                    }
                    std::cout << " | ";
                }
                std::cout << " \n ";
            }
            std::cout << std::endl;
            
        }

        void addBlockCount(ivec3 location) {
            blockCount++;
            blockCountX.at(location.x - boundsMin.x)++;
            blockCountY.at(location.y - boundsMin.y)++;
            blockCountZ.at(location.z - boundsMin.z)++;
        }

        void removeBlockCount(ivec3 location) {
            blockCount--;
            blockCountX.at(location.x - boundsMin.x)--;
            blockCountY.at(location.y - boundsMin.y)--;
            blockCountZ.at(location.z - boundsMin.z)--;
        }

        //assumes its not replacing a block. handles tracking and collider
        void placeBlockNoUpdate(ivec3 location,Block& block,BlockPlaceInfo info = BlockPlaceInfo()) {
            addBlockCount(location);
            size_t index = getIndex(location);
            auto state = block.onPlace(this,location,info);
            BlockPaletteEntry entry{&block,state};
            blockDataArray.at(index) = BlockData(paletteEntryToID(entry),info.attached);

            if(info.attached && !isStatic) {
                setIsStatic(true);
            }

            addBlockCollider(index,location);

            //printIDList();
        }

        void addBlockCollider(size_t index,ivec3 location) {
            physicsOldCOM = physicsShape->GetCenterOfMass(); //temporary until custom shape (hopefully)
            auto shape = new JPH::BoxShape(JPH::Vec3(0.5f,0.5f,0.5f));
            physicsShape->AddShape(Physics::toJoltVec((vec3)location),JPH::Quat::sIdentity(),shape,0,UINT_MAX);
            blockDataArray[index].shapeOpt = static_cast<JPH::Shape*>(shape);
            physicsShape->AdjustCenterOfMass();
            physicsShapeChanged = true;
        }

        // make the bounds
        void boundsEncapsulate(ivec3 location) {
            if(!isInsideBounds(location)) {
                vec3 newMin = glm::min(boundsMin,location);
                vec3 newMax = glm::max(boundsMax,location);
                setBounds(newMin,newMax);
            }
        }

        // places a block, must be valid. can't be used for destruction. replaces existing block. Handles updating mesh and stuff
        void placeBlock(ivec3 location,Block* block,BlockPlaceInfo placeInfo = BlockPlaceInfo()) {
            if(block == nullptr) {
                Debug::warn("tried to place null block");
                return;
            }

            boundsEncapsulate(location);
            
            removeBlockNoUpdate(location);

            
            
            placeBlockNoUpdate(location,*block,placeInfo);

            
            //printIDList();

            //recalculatePivot();
            generateMesh();
        }

        void calculateBreakup(World* world,ivec3 location) {
            std::vector<int> blockGroups(blockDataArray.size(),0);
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
            for (int z = boundsMin.z; z <= boundsMax.z; z++)
            {
                for (int y = boundsMin.y; y <= boundsMax.y; y++)
                {
                    for (int x = boundsMin.x; x <= boundsMax.x; x++)
                    {
                        i++; // so continues work
                        int blockGroup = blockGroups.at(i);
                        if(blockGroup != 0) {

                            if(blockGroup == originalConstructionGroup) {
                                auto blockData = blockDataArray.at(i);

                                if(blockData.attached) {
                                    setIsStatic(true);
                                    std::cout << blockGroup << " is attached" << std::endl;
                                }
                                continue;
                            }

                            vec3 worldLocation = transformPoint(vec3(x,y,z));
                            if(constructions[blockGroup-1] == nullptr) {
                                if(originalConstructionGroup == 0) {
                                    originalConstructionGroup = blockGroup;
                                    constructions[blockGroup-1] = this;
                                    auto blockData = blockDataArray.at(i);
                                    if(!blockData.attached) {
                                        setIsStatic(false);
                                    }
                                    continue;
                                } else {
                                    constructions[blockGroup-1] = world->spawn(Construction::makeInstance(material,worldLocation,rotation)).get();
                                    constructions[blockGroup-1]->body.setVelocity(body.getVelocity()); //match velocities
                                }
                            }
                            auto& newConstruction = *constructions[blockGroup-1];
                            ivec3 newLocalLocation = glm::round(newConstruction.inverseTransformPoint(worldLocation));
                            newConstruction.boundsEncapsulate(newLocalLocation);
                            int newIndex = newConstruction.getIndex(newLocalLocation);
                            auto blockData = blockDataArray.at(i);

                            if(blockData.attached) {
                                newConstruction.setIsStatic(true);
                                std::cout << blockGroup << " is attached" << std::endl;
                            }

                            auto blockEntry = blockPalette.at(blockData.id); // we actually do want to copy it here

                            newConstruction.addBlockCount(newLocalLocation);
                            
                            blockData.id = newConstruction.paletteEntryToID(blockEntry); // the IDs will need to change for this. Maybe just copying the entire palette is better and then removing the blocks?
                            newConstruction.blockDataArray.at(newIndex) = blockData;
                            
                            newConstruction.addBlockCollider(newIndex,newLocalLocation);

                            removeBlockNoUpdate(vec3(x,y,z)); // remove the original
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
            if(getBlock(location).block == nullptr) {
                //std::cout << "block is null" << std::endl;
                return;
            }
            int index = getIndex(location);
            if(blockGroups[index] >= group) {
                //std::cout << "group is " << group << std::endl;
                return;
            }
            //Debug::drawCube(transformPoint(location),vec3(1),rotation,debugGroupColors[group-1],1);
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

            bool minSet = false;
            for (size_t i = 0; i < blockCountX.size(); i++)
            {
                if(blockCountX[i] != 0) {
                    newMax.x = i + boundsMin.x;
                    if(!minSet) {
                        minSet = true;
                        newMin.x = i + boundsMin.x;
                    }
                }
            }
            minSet = false;
            for (size_t i = 0; i < blockCountY.size(); i++)
            {
                if(blockCountY[i] != 0) {
                    newMax.y = i + boundsMin.y;
                    if(!minSet) {
                        minSet = true;
                        newMin.y = i + boundsMin.y;
                    }
                }
            }
            minSet = false;
            for (size_t i = 0; i < blockCountZ.size(); i++)
            {
                if(blockCountZ[i] != 0) {
                    newMax.z = i + boundsMin.z;
                    if(!minSet) {
                        minSet = true;
                        newMin.z = i + boundsMin.z;
                    }
                }
            }
            if(newMin != boundsMin || newMax != boundsMax) {
                setBounds(newMin,newMax);
            }
            
        }

        // reduces bounds, call when block broken
        void recalculateBoundsFromBreak(ivec3 location) {

            // TODO should probably accumulate bounds change but its probably literally not an issue at all. It would have to be detached anyway
            ivec3 newMin = boundsMin;
            ivec3 newMax = boundsMax;
            ivec3 indices = location - boundsMin;
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
            if(newMin != boundsMin || newMax != boundsMax) {
                setBounds(newMin,newMax);
            }
        }

        void setMoveControl(vec3 move) {
            moveControl = glm::normalize(move);
        }

        void setTurnControl(vec3 turn) {
            turnControl = turn;
        }


        size_t getIndex(ivec3 location) {
            ivec3 fromMin = location - boundsMin;
            ivec3 size = boundsMax - boundsMin;
            size.x += 1;
            size.y += 1;
            size.z += 1;
            int i = std::max(fromMin.x + fromMin.y * size.x + fromMin.z * size.y * size.x,0);
            return std::min((size_t)i,blockDataArray.size()-1); //I think this wont cause issues with out of bounds stuff
        }

        BlockData getBlockData(ivec3 location) {
            if(!isInsideBounds(location)) {
                return BlockData(0);
            }
            auto data = blockDataArray.at(getIndex(location));
            return data;
        }

        BlockPaletteEntry& getBlock(ivec3 location) {
            
            auto data = getBlockData(location);
            return blockPalette.at(data.id);
        }

        vec3 blockCenterLocal(ivec3 location) {
            return vec3(location.x+0.5f,location.y+0.5f,location.z+0.5f);
        }

        bool isInsideBounds(ivec3 location) {
            if(location.x > boundsMax.x) return false;
            if(location.y > boundsMax.y) return false;
            if(location.z > boundsMax.z) return false;

            if(location.x < boundsMin.x) return false;
            if(location.y < boundsMin.y) return false;
            if(location.z < boundsMin.z) return false;
            return true;
        }

        void addStepCallback(ivec3 location) {
            stepCallbacks[Location(location)] = true;
        }

        void createBlockMap(map<Location,BlockData>& blockMap) {
            int i = 0;
            for (int z = boundsMin.z; z <= boundsMax.z; z++)
            {
                for (int y = boundsMin.y; y <= boundsMax.y; y++)
                {
                    for (int x = boundsMin.x; x <= boundsMax.x; x++)
                    {
                        if(blockDataArray[i].id != 0) {
                            blockMap[Location(x,y,z)] = blockDataArray[i];
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
            ptr->updateLastTransform();
            ptr->material = material;
            return std::unique_ptr<Construction>(ptr);
        }

        static std::unique_ptr<Construction>    makeInstance(Material material,Block* block,vec3 position,quat rotation = glm::identity<quat>(),bool attached = false) {
            auto ptr = makeInstance(material,position,rotation);
            BlockPlaceInfo info;
            info.attached = attached;
            ptr->placeBlock(ivec3(0),block,info);
            return ptr;
        }

        data_ActorType getActorDataType() {
            return data_ActorType::CONSTRUCTION;
        }

        virtual std::vector<std::uint8_t> createSaveBuffer() {
            auto data = save();
            auto buf = cista::serialize(data);
            return buf;
        }

        data_Construction save() {
            data_Construction data;
            data.actor = Actor::save();
            data.body = body.save();
            data.isStatic = isStatic;
            data.boundsMin.set(boundsMin);
            data.boundsMax.set(boundsMax);
            data.palette.reserve(blockPalette.size());
            for(auto& entry : blockPalette) {
                data.palette.push_back(entry.save());
            }
            data.blocks.reserve(blockDataArray.size());
            for(auto& block : blockDataArray) {
                data.blocks.push_back(block.save());
            }
            for(auto& pair : stepCallbacks) {
                if(pair.second) {
                    data_ivec3 data_pos;
                    data_pos.set(pair.first.asVec3());
                    data.stepCallbacks.push_back(data_pos); 
                }
            }
            //data.body.angularVelocity = body->GetAngularVelocity();
            return data;
        }

        void load(const data_Construction& data,DataLoader& loader) {
            Actor::load(data.actor);
            body.load(data.body);
            isStatic = data.isStatic;
            boundsMin = data.boundsMin.toVec3();
            boundsMax = data.boundsMax.toVec3();
            
            blockPalette.clear();
            blockPalette.reserve(data.palette.size());
            for(auto& data_entry : data.palette) {
                BlockPaletteEntry entry;
                entry.load(data_entry,loader);
                blockPalette.push_back(entry);
            }
            
            blockDataArray.clear();
            blockDataArray.reserve(data.blocks.size());
            blockCount = 0;
            // similar to resizing the bounds :shrug: maybe make a function?
            blockCountX.clear(); // this kinda repeated code could probably be written in a more elegant way but Idk how so :shrug:
            blockCountX.resize((boundsMax.x - boundsMin.x) + 1);
            blockCountY.clear();
            blockCountY.resize((boundsMax.y - boundsMin.y) + 1);
            blockCountZ.clear();
            blockCountZ.resize((boundsMax.z - boundsMin.z) + 1);
            size_t i = 0;
            for (int z = boundsMin.z; z <= boundsMax.z; z++)
            {
                for (int y = boundsMin.y; y <= boundsMax.y; y++)
                {
                    for (int x = boundsMin.x; x <= boundsMax.x; x++)
                    {
                        if(i >= data.blocks.size()) {
                            Debug::warn("tried to load invalid construction data (blockdata too large for bounds)");
                            return;
                        }
                        auto data_block = data.blocks[static_cast<unsigned int>(i)];
                        if(data_block.id != 0) {

                            BlockData block;
                            block.load(data_block);
                            //std::cout << "<" << x << "," << y << "," << z <<  ">:" << blockMap.at(Location(x,y,z)) << std::endl;
                            blockDataArray.push_back(block);
                            auto entry = blockPalette.at(block.id);
                            if(entry.block != nullptr) {
                                addBlockCollider(i,ivec3(x,y,z));
                                entry.block->onLoad(this,ivec3(x,y,z),entry.storage);
                                blockCount++;
                                blockCountX[x - boundsMin.x]++;
                                blockCountY[y - boundsMin.y]++;
                                blockCountZ[z - boundsMin.z]++;
                            }
                        } else {
                            std::cout << "empty block " << std::endl;
                            blockDataArray.push_back(BlockData());
                        }
                        
                        i++;
                    }
                }
            }
            stepCallbacks.clear();
            for(auto& pos : data.stepCallbacks) {
                addStepCallback(pos.toVec3());
            }
            generateMesh();
        }

        static std::unique_ptr<Actor> makeInstanceFromSave(data_Construction& data,Material material,DataLoader& loader) {
            auto actor = makeInstance(material,vec3(0.0f));
            actor->load(data,loader);
            std::cout << "LOADING CONSTRUCTION ACTOR" << std::endl;

            return actor;
        }
};