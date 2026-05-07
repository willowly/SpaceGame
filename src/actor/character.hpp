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
#include "actor/item-actor.hpp"
#include "item/recipe.hpp"
#include "components/inventory.hpp"

#include "physics/jolt-conversions.hpp"

#include "interface/actor/actor-widget.hpp"
#include "interface/menu-object.hpp"

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>

#include "physics/jolt-userdata.hpp"
#include "components/camera-shake.hpp"

#include "interface/item-slot-interact-options.hpp"

#include "actor/item-actor.hpp"

#include "components/character-body.hpp"

#include "persistance/actor/data-character.hpp"

class Character : public Actor {


    public: 

        // Prototype constructors
        
        // enum Action {
        //     Neutral,
        //     InMenu
        // } action = Action::Neutral;

        vec3 currentCameraPosition = {};
        vec3 lastCameraPosition = {};

        quat currentCameraRotation = {};
        quat lastCameraRotation = {};


        // prototype
        float moveSpeed = 5.0f;
        float lookSensitivity = 5;
        float height = 1.0f;
        float radius = 0.4f;
        float groundAcceleration = 15;
        float groundDecelleration = 20;
        float airAcceleration = 3;
        float jumpForce = 3;
        float craftSpeed = 1;
        float rotationSpeed = 90;
        float rotationAcceleration = 5;
        float itemDropDistance = 4;
        vec3 thirdPersonCameraOffset = vec3(1,0.5f,10);
        vec3 thirdPersonCameraRot = {};
        float inputBuffer = 0.05f; // 3 frames
        float coyoteTime = 0.1f; // 6 frames
        float cameraClearRadius = 0.2f; // the distance away from a wall that the camera should be
        
        // inputs
        bool clickInput = false;
        bool interactInput = false;
        bool dropInput = false;
        float rotationInput = 0;
        bool brakeInput = false;
        float jumpInput = 0; // if above 0, the input is active. set to 0 to consume. for buffering inputs

        // instance
        float lookPitch = 0;
        bool thirdPerson = false;
        bool noClip = false;
        vec3 moveInput = {};
        vec3 turnInput = {}; //temp for constructions
        int selectedTool = 0;
        static const int toolbarSize = 9;
        std::array<ItemStack,toolbarSize> toolbar = {};
        float groundedTimer = 0; // equal to coyote time when on ground, goes to 0.

        bool flying = true;

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

        ~Character() override = default;


        Inventory inventory;

        ItemStack craftingStack;

        ItemStack cursorStack;

        std::unique_ptr<MenuObject> openMenuObject = nullptr;

        std::vector<Recipe*> recipes;

        Recipe* currentRecipe = nullptr;
        float recipeTimer = 0; //

        bool underGravity = false;

        Character(const Character& character) :
            moveSpeed(character.moveSpeed),
            lookPitch(character.lookPitch),
            lookSensitivity(character.lookSensitivity),
            height(character.height),
            radius(character.radius),
            groundAcceleration(character.groundAcceleration),
            groundDecelleration(character.groundDecelleration),
            airAcceleration(character.airAcceleration),
            jumpForce(character.jumpForce),
            recipes(character.recipes),
            inputBuffer(character.inputBuffer),
            coyoteTime(character.coyoteTime),
            heldItemData(HeldItemData{}),
            cameraClearRadius(character.cameraClearRadius),
            Actor(character)
        {
            
        }

        CameraShake shake;

        bool inMenu = false;

        ActorWidget<Character>* widget = nullptr;


        CharacterBody body;

        vec3 angularVelocity = vec3(0.0f);


        void addRenderables(Vulkan* vulkan,float dt,float interpolation) override {
            if(ridingConstruction != nullptr) return;
            
            if(!toolbar[selectedTool].isEmpty()) {
                heldItemData.animationTimer += dt;
                toolbar[selectedTool].item->addRenderablesHeld(vulkan,*this,dt,interpolation);
            }
            RenderingSettings settings;
            settings.mainPass = thirdPerson;
            shake.step(dt);
            if(model == nullptr) return; //if no model, nothing to render :)
            model->addToRender(vulkan,material,getInterpolatedPosition(interpolation),getInterpolatedRotation(interpolation),vec3(modelScale),settings);
        }

        void spawn(World* world) override {

            body.generateCollisionEvents = true;

            JPH::Shape::ShapeResult result;
            auto capsuleSettings = JPH::CapsuleShapeSettings(height/2,radius);
            auto bodySettings = body.getDefaultCharacterSettings(this,new JPH::CapsuleShape(capsuleSettings,result),position,rotation);

            // bodySettings.mUserData = ActorUserData(this).asUInt();
            
            body.spawn(world,this,bodySettings);


        }

        void prePhysics(World* world) override {

            // turn into applyGravity function

            // body.prePhysics(world,position,rotation);
            // if(ridingConstruction == nullptr) {
            //     world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::PLAYER);
            // } else {
            //     world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::DISABLED);
            // }
            // Debug::drawCube(Physics::toGlmVec(bounds.GetCenter()),Physics::toGlmVec(bounds.GetSize()),glm::identity<quat>(),Color::green,0.01f);
        }

        void postPhysics(World* world) override {
            //body.postPhysics(world,position,rotation);
            //auto velocity = body.getVelocity();
            //physicsCharacter->PostSimulation(0.01f); // small number to be on the floor
        }

        void collisionStart(World* world,const Collision& contact) override {

            if(contact.otherActor == nullptr) return;

            auto itemActor = dynamic_cast<ItemActor*>(contact.otherActor);
            if(itemActor != nullptr) {
                give(itemActor->stack);
                itemActor->destroy(world);
            }
        }

        void groundLerpVelocity(float& v,float target,float dt) {
            if(target == 0) {
                v = MathHelper::lerp(v,target,groundDecelleration*dt);
            } else {
                if(abs(v) != abs(target)) v = 0;
                v = MathHelper::lerp(v,target,groundAcceleration*dt);
            }
        }

        vec3 getVelocity() {
            if(ridingConstruction != nullptr) {
                return ridingConstruction->getVelocity();
            } else {
                return body.getVelocity();
            }
        }

        void setUpVector(vec3 up) {
            vec3 basisY = glm::normalize(up);
            vec3 basisX = glm::cross(basisY,transformDirection(vec3(0,0,1)));
            vec3 basisZ = glm::cross(basisY,basisX);
            rotation = glm::quatLookAt(basisZ,basisY);
        }

        void step(World* world,float dt) override {

            updateLastTransform();

            auto velocity = body.getVelocity();
            if(ridingConstruction != nullptr) {
                
                // turn camera
                //angularVelocity.z = MathHelper::lerp(angularVelocity.z,rotationInput * rotationSpeed,rotationAcceleration * dt);
                

                if(brakeInput) {
                    ridingConstruction->setMoveControl(ridingConstruction->inverseTransformDirection(MathHelper::clampLength(-ridingConstruction->getVelocity(),1)));
                } else {
                    ridingConstruction->setMoveControl(ridingConstructionRotation  * glm::angleAxis(glm::radians(180.0f),vec3(0,1,0)) * moveInput); //we have to turn around bc we are facing negative Z
                }

                // rotation *= glm::angleAxis(glm::radians(lookPitch),vec3(-1,0,0));
                // lookPitch = 0;
                
                // // this big rotation could be factored out by just getting the "correct sitting rotation"
                // auto delta = glm::inverse(ridingConstruction->getRotation() * ridingConstructionRotation * glm::quat(glm::radians(vec3(0.0f,180.0f,0.0f)))) * rotation;

                // vec3 deltaEuler = glm::degrees(glm::eulerAngles(delta));
                auto constructionVelocity = ridingConstruction->inverseTransformDirection(ridingConstruction->getAngularVelocity());

                auto worldConstructionAngularVelocityRadians = glm::radians(ridingConstruction->getAngularVelocity());
                // kinda janky but to move the player with the construction
                rotation += dt * 0.5f * glm::quat(0,worldConstructionAngularVelocityRadians.x,worldConstructionAngularVelocityRadians.y,worldConstructionAngularVelocityRadians.z) * rotation;
                rotation = glm::normalize(rotation);
                
                // //lookPitch -= constructionVelocity.x * dt;
                // deltaEuler.x *= -1; // I have no idea why this works :( 
                // deltaEuler.z *= -1;

                //rotation = glm::angleAxis(glm::radians(ridingConstruction->getAngularVelocity()/z) * dt,ridingConstruction->transformDirection(vec3(0,0,-1))) * rotation;

                if(turnInput.x == 0) {
                    turnInput.x = -constructionVelocity.x;
                    if(abs(turnInput.x) > 1) {
                        turnInput.x = glm::sign(turnInput.x);
                    }
                }
                if(turnInput.y == 0) {
                    turnInput.y = -constructionVelocity.y;
                    if(abs(turnInput.y) > 1) {
                        turnInput.y = glm::sign(turnInput.y);
                    }
                }
                if(turnInput.z == 0) {
                    turnInput.z = -constructionVelocity.z;
                    if(abs(turnInput.z) > 1) {
                        turnInput.z = glm::sign(turnInput.z);
                    }
                }

                // if(deltaEuler.y != 0) {

                //     auto maxAccelerationYaw = ridingConstruction->getMaxAngularAccelerationYaw();

                //     // solve to make the next stop distance closer to the actual distance
                //     turnInput.y = (sign(deltaEuler.y) * sqrt(abs(deltaEuler.y) * 2*maxAccelerationYaw)-constructionVelocity.y) / (maxAccelerationYaw * dt);
                    
                //     if(abs(turnInput.y) > 1) {//clamp to -1,1
                //         turnInput.y = glm::sign(turnInput.y);
                //     }
                // }

                // if(deltaEuler.x != 0) {

                //     auto maxAccelerationPitch = ridingConstruction->getMaxAngularAccelerationPitch();

                //     // solve to make the next stop distance closer to the actual distance
                //     turnInput.x = (sign(deltaEuler.x) * sqrt(abs(deltaEuler.x) * 2*maxAccelerationPitch)-constructionVelocity.x) / (maxAccelerationPitch * dt); 
                    
                //     if(abs(turnInput.x) > 1) { //clamp to -1,1
                //         turnInput.x = glm::sign(turnInput.x);
                //     }
                // }

                // if(deltaEuler.z != 0) {

                //     auto maxAccelerationRoll = ridingConstruction->getMaxAngularAccelerationRoll();

                //     // solve to make the next stop distance closer to the actual distance
                //     turnInput.z = (sign(deltaEuler.z) * sqrt(abs(deltaEuler.z) * 2*maxAccelerationRoll)-constructionVelocity.z) / (maxAccelerationRoll * dt); 
                    
                //     if(abs(turnInput.z) > 1) { //clamp to -1,1
                //         turnInput.z = glm::sign(turnInput.z);
                //     }
                // }
                
                ridingConstruction->setTurnControl(turnInput);
                //std::cout << "deltaEuler:" << deltaEuler.z << " velocity: "<< constructionVelocity.z << std::endl;

                position = ridingConstruction->transformPoint(ridingConstructionPoint);
                //setUpVector(ridingConstruction->transformDirection(vec3(0.0f,1.0f,0.0f)));

                if(interactInput) {
                    dismount();
                }
            } else {

                auto gravity = world->getGravityVector(position);
                
                if(flying) gravity = vec3(0);


                if(glm::length(gravity) > 0.05f) {
                    underGravity = true;
                    setUpVector(-gravity);
                } else {
                    underGravity = false;
                }

                if(body.getGroundState() == JPH::CharacterBase::EGroundState::OnGround) {
                    groundedTimer = coyoteTime;
                }

                vec3 relativeVelocity = inverseTransformDirection(velocity);
                vec3 moveXZ = vec3(moveInput.x,0,moveInput.z);
                if(glm::length(moveXZ) != 0) moveXZ = glm::normalize(moveXZ);
                vec3 targetVelocity = moveXZ * moveSpeed;
                if(flying) {
                    targetVelocity.y = moveInput.y * moveSpeed;
                } else {
                    targetVelocity.y = relativeVelocity.y;
                }
                
                // std::cout << "player move input: " << StringHelper::toString(moveInput) << std::endl;
                if(groundedTimer == 0) {
                    relativeVelocity = MathHelper::lerp(relativeVelocity,targetVelocity,airAcceleration*dt);
                } else {
                    relativeVelocity = MathHelper::lerp(relativeVelocity,targetVelocity,groundAcceleration*dt);
                }

                

                velocity = transformDirection(relativeVelocity);

                //velocity += world->getGravityVector(position) * dt;
                
                if(!underGravity) {
                    // there might be soe 
                    angularVelocity.z = MathHelper::lerp(angularVelocity.z,rotationInput * rotationSpeed,rotationAcceleration * dt);
                    angularVelocity.x = MathHelper::lerp(angularVelocity.x,fmin(abs(lookPitch),rotationSpeed) * sign(lookPitch),rotationAcceleration * dt);
                    //angularVelocity.x = fmax(abs(angularVelocity.x),abs(lookPitch))
                    lookPitch -= angularVelocity.x * dt;
                    vec3 eyePos = getEyePosition();
                    rotation = glm::angleAxis(glm::radians(angularVelocity.z) * dt,transformDirection(vec3(0,0,-1))) * rotation;
                    rotation = glm::angleAxis(glm::radians(angularVelocity.x) * dt,transformDirection(vec3(-1,0,0))) * rotation;
                    position = eyePos - transformDirection(vec3(0,height*0.5f,0)); // to rotate around eye
                }

                if(!inMenu) {
                    if(interactInput) {
                        interact(world);
                    }
                    auto& selectedStack = toolbar.at(selectedTool);
                    if(!selectedStack.isEmpty()) {
                        selectedStack.item->step(world,*this,toolbar.at(selectedTool),dt);
                        heldItemData.actionTimer += dt;
                        if(dropInput) {
                            selectedStack.amount--;
                            auto droppedItemStack = ItemStack(selectedStack.item,1,selectedStack.storage);
                            world->spawn(ItemActor::makeInstance(droppedItemStack,getEyePosition() + getEyeDirection() * itemDropDistance,getEyeRotation()));
                        }
                    }
                    
                }

                clickInput = false;
                interactInput = false;
                dropInput = false;

                
                if(jumpInput > 0 && groundedTimer > 0) {
                    vec3 relativeVelocity = inverseTransformDirection(velocity);
                    relativeVelocity.y = jumpForce;
                    velocity = transformDirection(relativeVelocity);
                    jumpInput = 0;
                    groundedTimer = 0;
                }

                jumpInput = MathHelper::moveTowards(jumpInput,0,dt);
                

                body.setVelocity(velocity);

                body.update(world,this,position,rotation,gravity,dt);

                auto actors = world->overlapSphere(position,3);
                for (auto actor : actors)
                {
                    auto itemActor = dynamic_cast<ItemActor*>(actor);
                    if(itemActor != nullptr) {
                        // should be an acceleration thingy idk
                        auto velocity = itemActor->body.getVelocity();
                        vec3 delta = (position - itemActor->getPosition());
                        velocity = glm::normalize(delta) * 10.0f;
                        itemActor->body.setVelocity(velocity);
                    }
                }
                

                // if(!noClip) {
                //     world->collideBasic(this,height,radius);
                // }
            }

            lastCameraPosition = currentCameraPosition;
            lastCameraRotation = currentCameraRotation;

            // camera
            if(ridingConstruction == nullptr && !thirdPerson) {
                currentCameraPosition = getEyePosition();
            } else {
                
                currentCameraPosition = position + getEyeRotation() * thirdPersonCameraOffset;
                vec3 delta = currentCameraPosition - getEyePosition();
                RaycastSettings settings;
                if(ridingConstruction != nullptr) {
                    settings.setIgnoreBody(ridingConstruction->getBodyID());
                } else {
                    settings.setIgnoreBody(body.getCharacter()->GetInnerBodyID());
                }
                auto hitOpt = world->raycast(Ray(getEyePosition(),delta),glm::length(delta),LayerMask::excludes({Layers::PLAYER,Layers::ITEM}),settings);
                if(hitOpt) {
                    auto hit = hitOpt.value().hit;
                    currentCameraPosition = hit.point + (getLookRay().direction * cameraClearRadius * (1-glm::max(glm::dot(hit.normal,delta),0.0f)));
                }
            }

            currentCameraRotation = getEyeRotation();

            if(currentRecipe != nullptr) {
                if(!craftingStackHasIngredients(*currentRecipe)) {
                    cancelCraft();
                } else {
                    recipeTimer += dt * craftSpeed;
                    if(recipeTimer > currentRecipe->time) {
                        craftNoCheck(*currentRecipe);
                        recipeTimer = 0;
                    }
                }
            }
        }

        void destroy(World* world) {
            body.destroy(world);
        }

        void ride(Construction* construction,ivec3 point,quat rotation) {
            if(ridingConstruction != nullptr) {
                dismount();
            }
            //body->
            ridingConstruction = construction;
            ridingConstructionPoint = point;
            ridingConstructionRotation = rotation;
            this->rotation = construction->getRotation() * ridingConstructionRotation * glm::quat(glm::radians(vec3(0.0f,180.0f,0.0f)));
            lookPitch = 0;
        }

        void dismount() {
            position += ridingConstruction->transformDirection(vec3(0,1,0));
            ridingConstruction->setMoveControl(vec3(0));
            ridingConstruction->setTurnControl(vec3(0));
            ridingConstruction = nullptr;
        }

        void interact(World* world) {
            if(ridingConstruction != nullptr) {
                dismount();
                return;
            }
            auto worldHitOpt = world->raycast(Ray(getEyePosition(),getEyeDirection()),10,LayerMask::excludes({Layers::PLAYER,Layers::ITEM}));
            if(worldHitOpt) {
                auto worldHit = worldHitOpt.value();
                Construction* construction = dynamic_cast<Construction*>(worldHit.actor);
                if(construction != nullptr) {
                    std::cout << "interacted with construction" << std::endl;
                    vec3 interactPointWorld = worldHit.hit.point - worldHit.hit.normal * 0.5f;
                    vec3 interactPointLocal = construction->inverseTransformPoint(interactPointWorld);
                    ivec3 interactPointInt = glm::round(interactPointLocal);
                    auto data = construction->getBlock(interactPointInt);
                    auto block = data.block;
                    auto storage = data.storage;
                    if(data.block != nullptr) {
                        block->onInteract(construction,interactPointInt,storage,*this);
                    }
                }
            }
        }

        //could later be abstracted to a controller
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
            turnInput = vec3(0,0,0);
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
                turnInput.z -= 1;
            }
            if(input.getKey(GLFW_KEY_E)) {
                rotationInput += 1;
                turnInput.z += 1;
            }

            if(input.getKeyPressed(GLFW_KEY_SPACE)) {
                jumpInput = inputBuffer;
            }

            // probably temp for 
            if(input.getKey(GLFW_KEY_UP)) {
                turnInput.x -= 1; //im not sure why this exists :shrug:
            }
            if(input.getKey(GLFW_KEY_DOWN)) {
                turnInput.x += 1;
            }
            if(input.getKey(GLFW_KEY_LEFT)) {
                turnInput.y += 1;
            }
            if(input.getKey(GLFW_KEY_RIGHT)) {
                turnInput.y -= 1;
            }

            // if(input.getKeyPressed(GLFW_KEY_F2)) {
            //     moveSpeed *= 0.2f;
            //     std::cout << "Move speed set to: " << moveSpeed << std::endl;
            // }
            // if(input.getKeyPressed(GLFW_KEY_F3)) {
            //     moveSpeed *= 5.0f;
            //     std::cout << "Move speed set to: " << moveSpeed << std::endl;
            // }

            if(input.getKeyPressed(GLFW_KEY_F)) {
                interactInput = true;
            }

            if(input.getKeyPressed(GLFW_KEY_G)) {
                dropInput = true;
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
            if(input.getKeyPressed(GLFW_KEY_T)) {
                shake.startShake();
            }

            //check for cheats access
            if(input.getKeyPressed(GLFW_KEY_F1)) {
                noClip = !noClip;
            }

            if(input.getKeyPressed(GLFW_KEY_X)) {
                flying = !flying;
            }

            if(!toolbar[selectedTool].isEmpty()) {
                toolbar[selectedTool].item->processInput(input);
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

            
            if(index == selectedTool) return; //dont do anything if its the same tool

            if(!toolbar[selectedTool].isEmpty()) {
                toolbar[selectedTool].item->unequip(*this);
            }
            selectedTool = index;
            if(!toolbar[selectedTool].isEmpty()) {
                toolbar[selectedTool].item->equip(*this);
                heldItemData.setAction(0); // reset actions
                
            }
            
            
        }

        void setToolbar(int index,ItemStack stack) {
            toolbar[index] = stack;
        }

        // for tools to do when they reduce count etc
        void refreshTool() {
            setCurrentTool(selectedTool);
        }

        void closeMenu() {
            inMenu = false;
            openMenuObject = nullptr;
            
            returnCursor();
        }

        void openMenu() {
            openMenu(nullptr);
        }

        void openMenu(std::unique_ptr<MenuObject> menuObject) {
            heldItemData.setAction(0);
            inMenu = true;
            openMenuObject = std::move(menuObject);
            
        }


        // we should turn this into a crafter module. tho it needs to be able to handle variables in storage too idk
        void startCraft(Recipe& recipe) {
            if(!hasIngredients(recipe)) {
                return; // dont start the craft
            }
            if(recipe.ingredients.size() != 1) {
                Debug::warn("player recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
                return;
            }
             //should have them so we dont need to check

            if(craftingStack.tryInsert(recipe.ingredients[0])) {
                take(recipe.ingredients[0]);
            } else {
                // try to take the item out
                if(!craftingStack.isEmpty()) {
                    give(craftingStack);
                    craftingStack.clear();
                    // try again
                    if(craftingStack.tryInsert(recipe.ingredients[0])) {
                        take(recipe.ingredients[0]);
                    }
                }
            }
            if(currentRecipe != &recipe) {
                currentRecipe = &recipe;
                recipeTimer = 0;
            }
        }

        void cancelCraft() {
            currentRecipe = nullptr;
            recipeTimer = 0;
        }

        void itemSlotHoverActions(DrawContext context,ItemStack& stack,ItemSlotInteractOptions options = {}) {
            if(!options.allowInsert && !cursorStack.isEmpty()) {
                return; // block insertion
            }
            if(!options.allowRemove && !stack.isEmpty()) {
                return; // block removal
            }
            if(context.mouseLeftClicked()) {
                stack = replaceCursor(stack);
                return;
            }
            // insert or take one TODO: IDK what to do lol
            if(context.mouseRightClicked()) {
                if(!cursorStack.isEmpty() && stack.canInsert(cursorStack)) {
                    // place one
                    cursorStack.amount--;
                    stack.tryInsert(ItemStack(cursorStack.item,1,cursorStack.storage));
                    return;
                } 
                if(!stack.isEmpty() && cursorStack.canInsert(stack)) {
                    stack.amount--;
                    cursorStack.tryInsert(ItemStack(stack.item,1,stack.storage));
                    return;
                }
                
            }
        }

        // returns the itemstack in cursor
        ItemStack replaceCursor(ItemStack stack) {
            if(stack.tryInsert(cursorStack)) {
                // insert
                cursorStack.clear();
                return stack;
            } else {
                //swap em
                ItemStack returnStack = cursorStack;
                cursorStack = stack;
                return returnStack;
            }
        }

        // when theres no item to replace
        ItemStack dropCursor() {
            ItemStack returnStack = cursorStack;
            cursorStack.clear();
            return returnStack;
        }

        void give(ItemStack stack) {
            for (auto& toolbarStack : toolbar)
            {
                if(toolbarStack.tryInsert(stack)) {
                    return;
                }
            }
            inventory.give(stack);
            
        }

        int take(ItemStack stack) {
            return take(stack.item,stack.amount);
        }

        int take(Item* item,int amount) {
            int amountTaken = 0;
            for (auto& toolbarStack : toolbar)
            {
                amountTaken += toolbarStack.take(ItemStack(item,amount));
                if(amountTaken >= amount) {
                    return amount;
                }
            }
            return inventory.take(ItemStack(item,amount - amountTaken)) + amountTaken;
            
        }

        // this is kinda janky :shrug:
        bool hasIngredients(Recipe& recipe) {
            for (auto& ingredient : recipe.ingredients)
            {
                // basically reduce the required by invenyory
                int requiredByInventory = ingredient.amount;
                for (auto toolbarStack : toolbar)
                {
                    if(toolbarStack.item == ingredient.item) {
                        if(toolbarStack.amount > ingredient.amount) {
                            requiredByInventory = 0;
                            break; // dont even need to check inventory
                        } else {
                            requiredByInventory -= toolbarStack.amount;
                        }
                    }
                }
                if(requiredByInventory > 0 && !inventory.has(ItemStack(ingredient.item,requiredByInventory))) {
                    return false;
                }
            }
            return true;
        }

        bool craftingStackHasIngredients(Recipe& recipe) {
            if(recipe.ingredients.size() != 1) {
                Debug::warn("player recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
                return false;
            }
            if(!craftingStack.has(recipe.ingredients[0])) return false;
            return true;
        }

        bool tryCraft(Recipe& recipe) {
            if(!craftingStackHasIngredients(recipe)) return false;
            craftNoCheck(recipe);

            return true;
        }

        void craftNoCheck(Recipe& recipe) {
            craftingStack.take(recipe.ingredients[0]);
            give(recipe.result);
        }

        void returnCursor() {
            give(cursorStack);
            cursorStack.clear();
        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,transformDirection(vec3(0,-1,0))) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void setCamera(Camera& camera,float interpolation) {
            camera.position = MathHelper::lerp(lastCameraPosition,currentCameraPosition,interpolation);
            camera.rotation = glm::slerp(lastCameraRotation,currentCameraRotation,interpolation) * shake.getRotation();
            
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
            return transformPoint(vec3(0,height/2.0f,0));
        }

        vec3 getEyePositionInterpolated(float interpolation) {
            return transformPointInterpolated(vec3(0,height/2.0f,0),interpolation);
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

        data_ActorType getActorDataType() {
            return data_ActorType::PLAYER;
        }

        virtual std::vector<std::uint8_t> createSaveBuffer() {
            auto data = save();
            auto buf = cista::serialize(data);
            return buf;
        }

        data_Character save() {
            data_Character data;
            data.actor = Actor::save();
            data.body = body.save();
            data.lookPitch = lookPitch;
            data.selectedTool = selectedTool;
            data.thirdPerson = thirdPerson;
            for (size_t i = 0; i < toolbarSize; i++)
            {
                data.toolbar.push_back(toolbar[i].save());
            }
            data.inventory = inventory.save();
            return data;
        }

        void load(const data_Character& data,DataLoader& loader) {
            Actor::load(data.actor);
            body.load(data.body);
            lookPitch = data.lookPitch;
            selectedTool = data.selectedTool;
            refreshTool();
            for (size_t i = 0; i < data.toolbar.size() && i < toolbarSize; i++)
            {
                toolbar[i].load(data.toolbar[i],loader);
            }
            inventory.load(data.inventory,loader);
            thirdPerson = data.thirdPerson;
        }

        static std::unique_ptr<Actor> makeInstanceFromSave(data_Character& data,Character* prototype,DataLoader& loader) {
            if(prototype == nullptr) throw std::runtime_error("prototype is null");
            auto actor = makeInstanceFromPrototype(prototype);
            actor->load(data,loader);

            std::cout << "LOADING PLAYER ACTOR" << std::endl;

            return actor;
        }

    protected:
        Character() : Actor() {
            
        }

};