#pragma once
#include "actor.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <format>
#include <string>
#include <iostream>
#include "helper/math-helper.hpp"
#include "helper/string-helper.hpp"
//#include "rigidbody-actor.hpp"
#include "engine/input.hpp"
#include "engine/world.hpp"
#include "construction.hpp"

#include "item/item-stack.hpp"
#include "item/recipe.hpp"
#include "components/inventory.hpp"

#include <GLFW/glfw3.h>

#include "physics/jolt-conversions.hpp"

#include "interface/actor/actor-widget.hpp"
#include "interface/menu-object.hpp"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>

#include "physics/jolt-userdata.hpp"


class Character : public Actor {


    public: 

        // Prototype constructors
        
        // enum Action {
        //     Neutral,
        //     InMenu
        // } action = Action::Neutral;

        vec3 lastPosition = {};


        // prototype
        float moveSpeed = 5.0f;
        float lookPitch = 0;
        float lookSensitivity = 5;
        float height = 1.0f;
        float radius = 0.4;
        float acceleration = 5;
        float jumpForce = 10;
        float craftSpeed = 1;
        float rotationSpeed = 90;
        float rotationAcceleration = 5;

        // inputs
        bool clickInput = false;
        bool interactInput = false;
        float rotationInput = 0;
        bool brakeInput = false;

        bool thirdPerson = false;

        bool noClip = false;

        vec3 moveInput = {};

        vec3 velocity = {};
        vec3 rotationVelocity = {};
        

        Item* currentToolItem = nullptr;
        int selectedTool = 0;

        Item* toolbar[9] = {};

        struct HeldItemData {
            float actionTimer;
            float animationTimer;
            int action;
            void setAction(int newAction) {
                action = newAction;
                actionTimer = 0;
                animationTimer = 0;
            }
        } heldItemData;

        Construction* ridingConstruction = nullptr;
        ivec3 ridingConstructionPoint = {};
        quat ridingConstructionRotation = {};

        vec3 thirdPersonCameraOffset = vec3(1,0.5,10);
        vec3 thirdPersonCameraRot = {};

        Inventory inventory;

        std::unique_ptr<MenuObject> openMenuObject;

        std::vector<Recipe*> recipes;

        Recipe* currentRecipe = nullptr;
        float recipeTimer; //

        bool underGravity = true;

        Character(const Character& character) {
            moveSpeed = character.moveSpeed;
            lookPitch = character.lookPitch;
            lookSensitivity = character.lookSensitivity;
            height = character.height;
            radius = character.radius;
            acceleration = character.acceleration;
            jumpForce = character.jumpForce;
        }

    

        bool inMenu;

        ActorWidget<Character>* widget;


        JPH::Body* body;


        void addRenderables(Vulkan* vulkan,float dt) {
            if(ridingConstruction != nullptr) return;
            ItemStack* currentTool = inventory.getStack(currentToolItem);
            if(currentTool != nullptr) {
                heldItemData.animationTimer += dt;
                currentTool->item->addRenderables(vulkan,*this,dt);
            }
            if(thirdPerson) {
                Actor::addRenderables(vulkan,dt);
            }
        }

        virtual void spawn(World* world) {

            JPH::Shape::ShapeResult result;
            auto capsuleSettings = JPH::CapsuleShapeSettings(height/2,radius);
            JPH::BodyCreationSettings bodySettings(new JPH::CapsuleShape(capsuleSettings,result), Physics::toJoltVec(position), Physics::toJoltQuat(position), JPH::EMotionType::Dynamic, Layers::MOVING);

            bodySettings.mGravityFactor = 0.0f;

            // bodySettings.mUserData = ActorUserData(this).asUInt();
            bodySettings.mObjectLayer = Layers::PLAYER;
            bodySettings.mAllowedDOFs = (JPH::EAllowedDOFs::TranslationX | JPH::EAllowedDOFs::TranslationY | JPH::EAllowedDOFs::TranslationZ);

            
            body = world->physics_system.GetBodyInterface().CreateBody(bodySettings);
            
            world->physics_system.GetBodyInterface().AddBody(body->GetID(),JPH::EActivation::Activate);

            body->SetLinearVelocity(Physics::toJoltVec(velocity));


        }

        virtual void prePhysics(World* world) {
            world->physics_system.GetBodyInterface().SetPosition(body->GetID(),Physics::toJoltVec(position),JPH::EActivation::DontActivate);
            world->physics_system.GetBodyInterface().SetRotation(body->GetID(),Physics::toJoltQuat(rotation).Normalized(),JPH::EActivation::DontActivate);
            body->SetLinearVelocity(Physics::toJoltVec(velocity));
            body->SetAngularVelocity(Physics::toJoltVec(vec3(0.0)));
            if(ridingConstruction == nullptr) {
                world->physics_system.GetBodyInterface().SetObjectLayer(body->GetID(),Layers::PLAYER);
            } else {
                world->physics_system.GetBodyInterface().SetObjectLayer(body->GetID(),Layers::DISABLED);
            }
            auto bounds = body->GetWorldSpaceBounds();
            // Debug::drawCube(Physics::toGlmVec(bounds.GetCenter()),Physics::toGlmVec(bounds.GetSize()),glm::identity<quat>(),Color::green,0.01f);
        }

        virtual void postPhysics(World* world) {
            lastPosition = position;
            position = Physics::toGlmVec(body->GetPosition());
            rotation = Physics::toGlmQuat(body->GetRotation());
            velocity = Physics::toGlmVec(body->GetLinearVelocity());
            //physicsCharacter->PostSimulation(0.01f); // small number to be on the floor
        }

        void step(World* world,float dt) {

            if(ridingConstruction != nullptr) {
                
                
                if(brakeInput) {
                    ridingConstruction->setMoveControl(ridingConstruction->inverseTransformDirection(MathHelper::clampLength(-ridingConstruction->getVelocity(),1)));
                } else {
                    ridingConstruction->setMoveControl(ridingConstructionRotation  * glm::angleAxis(glm::radians(180.0f),vec3(0,1,0)) * moveInput); //we have to turn around bc we are facing negative Z
                }
                rotationVelocity.z = MathHelper::lerp(rotationVelocity.z,rotationInput * rotationSpeed,acceleration * dt); //maybe later we will feed this right into the construction
                ridingConstruction->setTargetRotation(getEyeRotation() * glm::inverse(ridingConstructionRotation) * glm::angleAxis(glm::radians(180.0f),vec3(0,1,0)));
                position = ridingConstruction->transformPoint(ridingConstructionPoint);
                if(interactInput) {
                    dismount();
                }
            } else {
                vec3 targetVelocity = rotation * moveInput * moveSpeed;
                // std::cout << "player move input: " << StringHelper::toString(moveInput) << std::endl;
                velocity = MathHelper::lerp(velocity,targetVelocity,acceleration*dt);
                position += velocity * dt;

                //velocity += world->getGravityVector(position) * dt;
                
                if(!underGravity) {
                    // there might be soe 
                    rotationVelocity.z = MathHelper::lerp(rotationVelocity.z,rotationInput * rotationSpeed,acceleration * dt);
                    rotationVelocity.x = MathHelper::lerp(rotationVelocity.x,fmin(abs(lookPitch),rotationSpeed) * sign(lookPitch),acceleration * dt);
                    //rotationVelocity.x = fmax(abs(rotationVelocity.x),abs(lookPitch))
                    lookPitch -= rotationVelocity.x * dt;
                    vec3 eyePos = getEyePosition();
                    rotation = glm::angleAxis(glm::radians(rotationVelocity.z) * dt,transformDirection(vec3(0,0,-1))) * rotation;
                    rotation = glm::angleAxis(glm::radians(rotationVelocity.x) * dt,transformDirection(vec3(-1,0,0))) * rotation;
                    position = eyePos - transformDirection(vec3(0,height,0)); // to rotate around eye
                }

                if(!inMenu) {
                    if(interactInput) {
                        interact(world);
                    }
                    ItemStack* currentTool = inventory.getStack(currentToolItem);
                    if(currentTool != nullptr) {
                        currentTool->item->step(world,*this,*currentTool,dt);
                        heldItemData.actionTimer += dt;
                    }
                }

                clickInput = false;
                interactInput = false;

                // if(!noClip) {
                //     world->collideBasic(this,height,radius);
                // }
            }

            if(currentRecipe != nullptr) {
                recipeTimer += dt * craftSpeed;
                if(recipeTimer > currentRecipe->time) {
                    if(inventory.tryCraft(*currentRecipe)) {
                        cancelCraft();
                    }
                }
            }

            

            

        }

        void ride(Construction* construction,ivec3 point,quat rotation) {
            if(ridingConstruction != nullptr) {
                dismount();
            }
            //body->
            thirdPerson = true;
            ridingConstruction = construction;
            ridingConstructionPoint = point;
            ridingConstructionRotation = rotation; //because the camera faces backwards
            //body->getCollider(0)->setIsTrigger(true);
        }

        void dismount() {
            //body->SetLayer(Layers::MOVING);
            position += ridingConstruction->transformDirection(vec3(0,1,0));
            ridingConstruction->resetTargets();
            ridingConstruction = nullptr;
            thirdPerson = false;
            //body->getCollider(0)->setIsSimulationCollider(true);
        }

        void interact(World* world) {
            if(ridingConstruction != nullptr) {
                dismount();
                return;
            }
            auto worldHitOpt = world->raycast(Ray(getEyePosition(),getEyeDirection()),10);
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    std::cout << "interacted with construction" << std::endl;
                    vec3 interactPointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 interactPointLocal = construction->inverseTransformPoint(interactPointWorld);
                    ivec3 interactPointInt = glm::round(interactPointLocal);
                    auto data = construction->getBlock(interactPointInt);
                    auto block = data.first;
                    auto state = data.second;
                    if(data.first != nullptr) {
                        block->onInteract(construction,interactPointInt,state,*this);
                    }
                }
            }
        }

        //could layer be abstracted to a controller
        void processInput(Input& input) {

            // eventually we want an enum for keys instead of using the defines
            

            if(inMenu) {
                processInputInventory(input);
            } else {
                processInputNormal(input);
            }
        }

        void processInputNormal(Input& input) {
            moveInput = vec3(0,0,0);
            if(input.getKey(GLFW_KEY_W)) {
                moveInput.z -= 1; //im not sure why this exists :shrug:
            }
            if(input.getKey(GLFW_KEY_S)) {
                moveInput.z += 1;
            }
            if(input.getKey(GLFW_KEY_A)) {
                moveInput.x -= 1;
            }
            if(input.getKey(GLFW_KEY_D)) {
                moveInput.x += 1;
            }
            if(input.getKey(GLFW_KEY_SPACE)) {
                moveInput.y += 1;
            }
            if(input.getKey(GLFW_KEY_C)) {
                moveInput.y -= 1;
            }
            rotationInput = 0;
            if(input.getKey(GLFW_KEY_Q)) {
                rotationInput -= 1;
            }
            if(input.getKey(GLFW_KEY_E)) {
                rotationInput += 1;
            }

            if(input.getKeyPressed(GLFW_KEY_F)) {
                interactInput = true;
            }

            brakeInput = input.getKey(GLFW_KEY_LEFT_SHIFT) || input.getKey(GLFW_KEY_RIGHT_SHIFT);

            if(input.getKeyPressed(GLFW_KEY_1)) {
                setCurrentTool(0);
            }
            if(input.getKeyPressed(GLFW_KEY_2)) {
                setCurrentTool(1);
            }
            if(input.getKeyPressed(GLFW_KEY_3)) {
                setCurrentTool(2);
            }
            if(input.getKeyPressed(GLFW_KEY_4)) {
                setCurrentTool(3);
            }
            if(input.getKeyPressed(GLFW_KEY_5)) {
                setCurrentTool(4);
            }
            if(input.getKeyPressed(GLFW_KEY_6)) {
                setCurrentTool(5);
            }
            if(input.getKeyPressed(GLFW_KEY_7)) {
                setCurrentTool(6);
            }
            if(input.getKeyPressed(GLFW_KEY_8)) {
                setCurrentTool(7);
            }
            if(input.getKeyPressed(GLFW_KEY_9)) {
                setCurrentTool(8);
            }
            if(input.getKeyPressed(GLFW_KEY_Z)) {
                thirdPerson = !thirdPerson;
            }

            //check for cheats access
            if(input.getKeyPressed(GLFW_KEY_F1)) {
                noClip = !noClip;
            }

            if(input.getKeyPressed(GLFW_KEY_X)) {
                underGravity = !underGravity;
            }

            ItemStack* currentTool = inventory.getStack(currentToolItem);
            if(currentTool != nullptr) {
                currentTool->item->processInput(input);
            }

            if(input.getKeyPressed(GLFW_KEY_TAB)) {
                openMenu();
            }
            moveMouse(input.getMouseDelta() * 0.01f);
        }

        void processInputInventory(Input& input) {
            moveInput = vec3(0,0,0);
            if(input.getKeyPressed(GLFW_KEY_TAB)) {
                closeMenu();
            }
            if(input.getKeyPressed(GLFW_KEY_F)) {
                closeMenu();
            }
            input.getMouseDelta();
        }

        void setCurrentTool(int index) {

            auto newTool = inventory.getStack(toolbar[index]);
            auto currentTool = inventory.getStack(currentToolItem);
            if(newTool == currentTool) return; //dont do anything if its the same tool

            if(currentTool != nullptr) {
                currentTool->item->unequip(*this);
            }
            selectedTool = index;
            if(newTool != nullptr) {
                newTool->item->equip(*this);
                heldItemData.setAction(0); // reset actions
                currentToolItem = newTool->item;
            } else {
                currentToolItem = nullptr;
            }
            
            
        }

        void setToolbar(int index,Item* item) {
            for(auto&& itemInBar : toolbar) {
                if(itemInBar == item) {
                    itemInBar = nullptr;
                }
            }
            toolbar[index] = item;
        }

        // for tools to do when they reduce count etc
        void refreshTool() {
            setCurrentTool(selectedTool);
        }

        void closeMenu() {
            inMenu = false;
            openMenuObject = nullptr;
        }

        void openMenu() {
            openMenu(nullptr);
        }

        void openMenu(std::unique_ptr<MenuObject> menuObject) {
            heldItemData.setAction(0);
            inMenu = true;
            openMenuObject = std::move(menuObject);
            
        }

        void startCraft(Recipe& recipe) {
            if(!inventory.hasIngredients(recipe)) {
                // dont even start it
                return;
            }
            currentRecipe = &recipe;
            recipeTimer = 0;
        }

        void cancelCraft() {
            currentRecipe = nullptr;
            recipeTimer = 0;
        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,transformDirection(vec3(0,-1,0))) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
            // if under gravitational forces...
                // lookPitch += delta.y * lookSensitivity;
                // if(lookPitch > 89.9f) lookPitch = 89.9f;
                // if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void setCamera(Camera& camera) {
            if(ridingConstruction == nullptr && !thirdPerson) {
                camera.position = getEyePosition();
                camera.rotation = getEyeRotation();
            } else {
                camera.position = position + getEyeRotation() * thirdPersonCameraOffset;
                camera.rotation = getEyeRotation();
            }
            
        }

        static std::unique_ptr<Character> makeDefaultPrototype() {
            auto ptr = new Character();
            return std::unique_ptr<Character>(ptr);
        }

        static std::unique_ptr<Character> makeInstance(Character* prototype,vec3 position = vec3(0),quat rotation = glm::identity<quat>()) {
            return makeInstanceFromPrototype<Character>(prototype,position,rotation);
        }

        glm::mat4 getTransform() {
            return Actor::getTransform();
        }

        vec3 getEyePosition() {
            return transformPoint(vec3(0,height,0));
        }

        quat getEyeRotation() {
            return rotation * glm::angleAxis(glm::radians(lookPitch),vec3(-1,0,0));
        }

        vec3 getEyeDirection() {
            return getEyeRotation() * vec3(0,0,-1);
        }

        Ray getLookRay() {
            return Ray(getEyePosition(),getEyeDirection());
        }

        bool playerStep() {
            return true;
        }

    protected:
        Character() : Actor() {
            
        }

};