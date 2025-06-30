#pragma once

#include "actor.hpp"
#include <glm/glm.hpp>
#include <map>

#include "engine/debug.hpp"
#include "rigidbody-actor.hpp"

#include <helper/string-helper.hpp>
#include <algorithm>

#include <block/block.hpp>

using glm::ivec3,glm::vec3;
using std::map;

class Construction : public RigidbodyActor {


    struct BlockData {
        Block* block = nullptr; //eventually this will be a list of blocks in the construction
        rp3d::Collider* collider = nullptr;
        BlockData(Block* block,rp3d::Collider* collider = nullptr) : block(block), collider(collider) {}
        BlockData() {}
    };

    ivec3 min = ivec3(0,0,0);
    ivec3 max = ivec3(0,0,0);
    std::vector<BlockData> blockData;

    vec3 targetVelocity;
    quat targetRotation;

    float thrustForce = 5;
    float turnForce = 0.5;

    

    public:

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

        Construction(Model* model,Material* material) : RigidbodyActor(model,material) {
            blockData.push_back(BlockData()); //starting block
        }
        Construction() : Construction(nullptr,nullptr) {
            
        }

        void step(float dt,World* world) {
            // just for testing
            std::cout << StringHelper::toString(targetVelocity) << std::endl;
            velocity = MathHelper::moveTowards(velocity,rotation * targetVelocity,thrustForce * dt);
            rotation = glm::slerp(rotation,targetRotation,turnForce * dt);
        }


        void render(Camera& camera,float dt) {
            int i = 0;
            for (int z = min.z; z <= max.z; z++)
            {
                for (int y = min.y; y <= max.y; y++)
                {
                    for (int x = min.x; x <= max.x; x++)
                    {
                        
                        if(blockData.size() > i && blockData[i].block != nullptr) {
                            blockData[i].block->model->render(glm::translate(transform(),vec3(x,y,z)),camera,*blockData[i].block->material);
                        } else {
                            // model->renderMode = Model::RenderMode::Wireframe;
                            // model->render(transformPoint(vec3(x,y,z)),camera,*Debug::getShader());
                            // model->renderMode = Model::RenderMode::Solid;
                        }
                        i++;
                    }
                }
            }

            //Debug::drawCube(position,vec
            
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

            // for(auto& pair : blockMap) {
            //     std::cout << "<" << pair.first.x << "," << pair.first.y << "," << pair.first.z <<  ">:" << pair.second << std::endl;
            // }

            int i = 0;
            blockData.clear();
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
                        } else {
                            //std::cout << "e,";
                            blockData.push_back(BlockData());
                        }
                        i++;
                    }
                }
            }
            //std::cout << std::endl;
            //Debug::info("Setting bounds to " + StringHelper::toString(newMin) + " " + StringHelper::toString(newMax),InfoPriority::LOW);
            // for(int block : blocks) {
            //     std::cout << block << ",";
            // }
            // std::cout << std::endl;
        }

        void setBlock(ivec3 location,Block* block) {
            if(!isInsideBounds(location)) {
                setBounds(glm::min(min,location),glm::max(max,location));
            }
            int index = getIndex(location);
            //Debug::info("index: " + std::to_string(index) + " " + StringHelper::toString(location),InfoPriority::LOW);
            if(block != nullptr) {
                auto collider = body->addCollider(common->createBoxShape(rp3d::Vector3(0.5f,0.5f,0.5f)),rp3d::Transform(PhysicsHelper::toRp3dVector(location),rp3d::Quaternion::identity()));
                collider->getMaterial().setBounciness(0.0);
                blockData[index] = BlockData(block,collider);
            } else {
                if(blockData[index].collider != nullptr) {
                    body->removeCollider(blockData[index].collider);
                }
                blockData[index] = BlockData();
            }
            
        }

        void setTargetVelocity(vec3 target) {
            targetVelocity = target;
        }

        void setTargetRotation(quat target) {
            targetRotation = target;
        }

        void resetTargets() {
            targetVelocity = vec3(0,0,0);
            targetRotation = rotation;
        }

        int getIndex(ivec3 location) {
            ivec3 fromMin = location - min;
            ivec3 size = max - min;
            size.x += 1;
            size.y += 1;
            size.z += 1;
            return fromMin.x + fromMin.y * size.x + fromMin.z * size.y * size.x;
        }

        Block* getBlock(ivec3 location) {
            if(!isInsideBounds(location)) {
                return nullptr;
            }
            return blockData[getIndex(location)].block;
        }

        void addCollisionShapes(rp3d::PhysicsCommon* common) {

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

        void addToPhysicsWorld(rp3d::PhysicsWorld* world,rp3d::PhysicsCommon* common) {
            RigidbodyActor::addToPhysicsWorld(world,common);
            body->setLinearDamping(0.1);
            body->setAngularDamping(3);
            //body->setType(rp3d::BodyType::STATIC);
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