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

using glm::ivec3,glm::vec3;
using std::unordered_map;

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


    std::vector<int> blockCountX;

    vec3 moveControl;
    quat targetRotation = glm::identity<quat>();
    float turnForce = 1.5;

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

            constexpr auto operator<=>(const Location&) const = default;

        };

        void step(World* world,float dt) {
            
            

            // i have no idea why z and x are inverted :shrug:
            // if(moveControl.z > 0) {
            //     applyForce(vec3(0,0,-1) * thrustForces[BlockFacing::FORWARD] * moveControl.z);
            // }
            // if(moveControl.z < 0) {
            //     applyForce(vec3(0,0,-1) * thrustForces[BlockFacing::BACKWARD] * moveControl.z);
            // }
            // if(moveControl.x < 0) {
            //     applyForce(vec3(-1,0,0) * thrustForces[BlockFacing::LEFT] * moveControl.x);
            // }
            // if(moveControl.x > 0) {
            //     applyForce(vec3(-1,0,0) * thrustForces[BlockFacing::RIGHT] * moveControl.x);
            // }
            // if(moveControl.y < 0) {
            //     applyForce(vec3(0,1,0) * thrustForces[BlockFacing::UP] * moveControl.y);
            // }
            // if(moveControl.y > 0) {
            //     applyForce(vec3(0,1,0) * thrustForces[BlockFacing::DOWN] * moveControl.y);
            // }
            // //velocity = MathHelper::moveTowards(velocity,rotation * targetVelocity,thrustForce * dt);
            // rotation = glm::slerp(rotation,targetRotation,turnForce * dt);
            
        }

        void addRenderables(Vulkan* vulkan,float dt) {
            size_t i = 0;
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        
                        if(blockData.size() > i && blockData[i].block != nullptr) {
                            quat rotation = getRotationFromFacing(blockData[i].state.facing);
                            glm::mat4 blockMatrix = glm::translate(getTransform(),vec3(x,y,z)) * glm::mat4(rotation);
                            blockData[i].block->model->addToRender(vulkan,blockData[i].block->material,blockMatrix);
                        } else {
                            // model->renderMode = Model::RenderMode::Wireframe;
                            // model->render(transformPoint(vec3(x,y,z)),camera,*Debug::getShader());
                            // model->renderMode = Model::RenderMode::Solid;
                        }
                        i++;
                    }
                }
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

        virtual void collideBasic(Actor* actor,float radius) {
            vec3 localActorPosition = inverseTransformPoint(actor->position);
            for (int z = floor(std::max((float)min.z,localActorPosition.z-radius)); z <= ceil(std::min((float)max.z,localActorPosition.z+radius)); z++)
            {
                for (int y = floor(std::max((float)min.y,localActorPosition.y-radius)); y <= ceil(std::min((float)max.y,localActorPosition.y+radius)); y++)
                {
                    for (int x = floor(std::max((float)min.x,localActorPosition.x-radius)); x <= ceil(std::min((float)max.x,localActorPosition.x+radius)); x++)
                    {
                        int i = getIndex(ivec3(x,y,z));
                        if(blockData[i].block != nullptr) {
                            auto contact_opt = Physics::intersectSphereBox(localActorPosition,radius,vec3(x,y,z),vec3(0.5f));
                            if(contact_opt) {
                                
                                auto contact = contact_opt.value();
                                Physics::resolveBasic(localActorPosition,contact);
                                
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

            for (size_t i = 0; i < blockCountX.size(); i++)
            {
                std::cout << blockCountX[i] << ", ";
            }
            
            std::cout << std::endl;
            
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

        static std::unique_ptr<Construction> makeInstance(vec3 position,quat rotation = glm::identity<quat>()) {
            auto ptr = new Construction();
            ptr->position = position;
            ptr->rotation = rotation;
            return std::unique_ptr<Construction>(ptr);
        }

        static std::unique_ptr<Construction> makeInstance(Block* block,vec3 position,quat rotation = glm::identity<quat>()) {
            auto ptr = makeInstance(position,rotation);
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