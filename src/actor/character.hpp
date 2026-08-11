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
// #include "rigidbody-actor.hpp"
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

#include "helper/event.hpp"

class Character : public Actor
{

public:
    inline static const string EVENT_UPDATE_HELD_ITEM = "update_held_item";

    // Prototype constructors

    // enum Action {
    //     Neutral,
    //     InMenu
    // } action = Action::Neutral;

    vec3 currentCameraPosition = {};
    vec3 lastCameraPosition = {};

    quat currentCameraRotation = glm::identity<quat>();
    quat lastCameraRotation = glm::identity<quat>();

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
    vec3 thirdPersonCameraOffset = vec3(1, 2.0f, 10);
    vec3 thirdPersonCameraRot = {};
    float inputBuffer = 0.05f;      // 3 frames
    float coyoteTime = 0.1f;        // 6 frames
    float cameraClearRadius = 0.2f; // the distance away from a wall that the camera should be
    bool inventoryDisabled = false;
    Mesh<Vertex> *model = nullptr;
    Material material = Material::none;
    float modelScale = 1;

    // inputs
    bool clickInput = false;
    bool interactInput = false;
    bool dropInput = false;
    bool targetInput = false;
    float rotationInput = 0;
    bool brakeInput = false;
    float jumpInput = 0; // if above 0, the input is active. set to 0 to consume. for buffering inputs

    // instance
    float lookPitch = 0;
    bool thirdPerson = false;
    bool noClip = false;
    vec3 moveInput = {};
    vec3 turnInput = {}; // temp for constructions
    float constructionTurnSensitivity = 0.1f;
    int selectedTool = 0;
    static const int toolbarSize = 9;
    std::array<ItemStack, toolbarSize> toolbar = {};
    float groundedTimer = 0; // equal to coyote time when on ground, goes to 0.
    bool alwaysRender = true;

    bool flying = true;

    struct HeldItemData
    {
        float actionTimer;
        float animationTimer;
        int action;
        quat lookOrientation = glm::identity<quat>(); // the rendered one, that gets lerped
        bool clickInput = false;
        bool clickHold = false;
        bool altClickInput = false;
        bool altClickHold = false;
        float scrollDelta;
        vec2 mouseDelta;
        Actor *attachedActor;
        void setAction(int newAction)
        {
            action = newAction;
            actionTimer = 0;
            animationTimer = 0;
        }
    } heldItemData;

    Construction *ridingConstruction = nullptr;
    ivec3 ridingConstructionPoint = {};
    quat ridingConstructionRotation = {};

    ~Character() override = default;

    Inventory inventory;

    ItemStack craftingStack;

    ItemStack cursorStack;

    std::unique_ptr<MenuObject> openMenuObject = nullptr;

    std::vector<Recipe *> recipes;

    Recipe *currentRecipe = nullptr;
    float recipeTimer = 0; //

    bool underGravity = false;

    ActorID targetedActorID = Invalid_ActorID;

    Character(const Character &character) : moveSpeed(character.moveSpeed),
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
                                            inventoryDisabled(character.inventoryDisabled),
                                            widget(character.widget),
                                            model(character.model),
                                            material(character.material),
                                            modelScale(character.modelScale),
                                            Actor(character)
    {
    }

    CameraShake shake;

    bool inMenu = false;

    ActorWidget<Character> *widget = nullptr;

    CharacterBody body;

    vec3 angularVelocity = vec3(0.0f);

    struct EventHeldItemChanged
    {
        Character *character = nullptr;
        ItemStack stack;
        int slot;
    };

    Event<EventHeldItemChanged> onHeldItemChanged;

    struct EventInventoryChanged
    {
        Character *character = nullptr;
        ItemStack stack;
        bool lose = false;
    };

    Event<EventInventoryChanged> onInventoryChanged;

    struct EventItemDropInput
    {
        Character *character = nullptr;
    };

    Event<EventItemDropInput> onItemDropInput;

    struct EventToolAction
    {
        Character *character = nullptr;
        Item *tool = nullptr;
        int actionEvent;
    };

    Event<EventToolAction> onToolAction;

    void addRenderables(Vulkan *vulkan, float dt, float interpolation) override
    {
        if (ridingConstruction != nullptr)
            return;

        if (!toolbar[selectedTool].isEmpty())
        {
            heldItemData.animationTimer += dt;
            toolbar[selectedTool].item->addRenderablesHeld(vulkan, *this, dt, interpolation);
        }
        RenderingSettings settings;
        settings.mainPass = thirdPerson || alwaysRender;
        shake.step(dt);
        if (model == nullptr)
            return; // if no model, nothing to render :)
        model->addToRender(vulkan, material, getInterpolatedPosition(interpolation), getInterpolatedRotation(interpolation), vec3(modelScale), settings);
    }

    void spawn(World *world) override
    {

        body.generateCollisionEvents = true;

        JPH::Shape::ShapeResult result;
        auto capsuleSettings = JPH::CapsuleShapeSettings(height / 2, radius);
        auto bodySettings = body.getDefaultCharacterSettings(this, new JPH::CapsuleShape(capsuleSettings, result), position, rotation);

        // bodySettings.mUserData = ActorUserData(this).asUInt();

        body.spawn(world, this, bodySettings);
    }

    void prePhysics(World *world) override
    {

        // turn into applyGravity function

        // body.prePhysics(world,position,rotation);
        // if(ridingConstruction == nullptr) {
        //     world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::PLAYER);
        // } else {
        //     world->physics_system.GetBodyInterface().SetObjectLayer(body.getBodyID(),Layers::DISABLED);
        // }
        // Debug::drawCube(Physics::toGlmVec(bounds.GetCenter()),Physics::toGlmVec(bounds.GetSize()),glm::identity<quat>(),Color::green,0.01f);
    }

    void postPhysics(World *world) override
    {
        // body.postPhysics(world,position,rotation);
        // auto velocity = body.getVelocity();
        // physicsCharacter->PostSimulation(0.01f); // small number to be on the floor
    }

    void collisionStart(World *world, const Collision &contact) override
    {

        if (contact.otherActor == nullptr)
            return;

        auto itemActor = dynamic_cast<ItemActor *>(contact.otherActor);
        if (itemActor != nullptr)
        {
            give(itemActor->stack);
            itemActor->destroy(world);
        }
    }

    void groundLerpVelocity(float &v, float target, float dt)
    {
        if (target == 0)
        {
            v = MathHelper::lerp(v, target, groundDecelleration * dt);
        }
        else
        {
            if (abs(v) != abs(target))
                v = 0;
            v = MathHelper::lerp(v, target, groundAcceleration * dt);
        }
    }

    ItemStack getHeldItemStack()
    {
        return toolbar.at(selectedTool);
    }

    vec3 getVelocity()
    {
        if (ridingConstruction != nullptr)
        {
            return ridingConstruction->getVelocity();
        }
        else
        {
            return body.getVelocity();
        }
    }

    void setUpVector(vec3 up)
    {
        auto oldRotation = rotation;
        vec3 oldLook = getEyeDirection();
        vec3 basisY = glm::normalize(up);
        vec3 basisX = glm::cross(basisY, transformDirection(vec3(0, 0, 1)));
        vec3 basisZ = glm::cross(basisY, basisX);
        rotation = glm::quatLookAt(basisZ, basisY);
        vec3 delta = glm::cross(oldLook, getEyeDirection());
        delta = inverseTransformDirection(glm::degrees(delta));
        lookPitch += delta.x;
    }

    void updateLastCameraPosition()
    {
        lastCameraPosition = currentCameraPosition;
        lastCameraRotation = currentCameraRotation;
    }

    void handleCamera(World *world)
    {
        updateLastCameraPosition();

        // camera
        if (ridingConstruction == nullptr && !thirdPerson)
        {
            currentCameraPosition = getEyePosition();
        }
        else
        {

            currentCameraPosition = position + getEyeRotation() * thirdPersonCameraOffset;
            vec3 delta = currentCameraPosition - getEyePosition();
            RaycastSettings settings;
            if (ridingConstruction != nullptr)
            {
                settings.setIgnoreBody(ridingConstruction->getBodyID());
            }
            else
            {
                settings.setIgnoreBody(body.getCharacter()->GetInnerBodyID());
            }
            auto hitOpt = world->raycast(Ray(getEyePosition(), delta), glm::length(delta), LayerMask::excludes({Layers::PLAYER, Layers::ITEM}), settings);
            if (hitOpt)
            {
                auto hit = hitOpt.value();
                currentCameraPosition = hit.point + (getLookRay().direction * cameraClearRadius * (1 - glm::max(glm::dot(hit.normal, delta), 0.0f)));
            }
        }

        currentCameraRotation = getEyeRotation();
    }

    void doConstructionControl(float dt)
    {
        if (brakeInput)
        {
            ridingConstruction->setMoveControl(ridingConstruction->inverseTransformDirection(MathHelper::clampLength(-ridingConstruction->getVelocity(), 1)));
        }
        else
        {
            ridingConstruction->setMoveControl(ridingConstructionRotation * glm::angleAxis(glm::radians(180.0f), vec3(0, 1, 0)) * moveInput); // we have to turn around bc we are facing negative Z
        }

        auto constructionAngularVelocity = ridingConstruction->getAngularVelocity();

        auto worldConstructionAngularVelocityRadians = glm::radians(ridingConstruction->getAngularVelocity());

        // kinda janky but to move the player with the construction
        // rotation += dt * 0.5f * glm::quat(0,0,0,ridingConstruction->inverseTransformDirection(constructionAngularVelocity).z) * rotation;
        // rotation = glm::normalize(rotation);
        // auto shipForward = ridingConstructionRotation * ridingConstruction->transformDirection(vec3(0,0,1.0f));

        // auto playerForward = getLookRay().direction;

        angularVelocity.z = MathHelper::lerp(angularVelocity.z, rotationInput * rotationSpeed, rotationAcceleration * dt);
        rotation = glm::angleAxis(glm::radians(angularVelocity.z) * dt, transformDirection(vec3(0, 0, -1))) * rotation;

        rotation *= glm::angleAxis(glm::radians(lookPitch), vec3(-1, 0, 0));
        lookPitch = 0;
        // setUpVector(ridingConstruction->transformDirection(vec3(0,1,0) * ridingConstructionRotation));

        auto delta = glm::inverse(ridingConstruction->getRotation()) * (rotation * glm::angleAxis(glm::radians(180.0f), vec3(0, 1, 0)) * glm::inverse(ridingConstructionRotation));

        // std::cout << "delta: " << StringHelper::toString(glm::degrees(glm::eulerAngles(ridingConstruction->getRotation()))) << " - " << StringHelper::toString(glm::degrees(glm::eulerAngles(rotation))) << " = " << StringHelper::toString(glm::degrees(glm::eulerAngles(delta))) << std::endl;

        // auto deltaRoll = shipRoll - playerRoll;

        auto kp = 50.0f;
        auto kd = 0.2f;

        vec3 turnControl = -vec3(delta.x, delta.y, delta.z) * kp;
        turnControl += -ridingConstruction->inverseTransformDirection(constructionAngularVelocity) * kd;
        // turnInput.y = -glm::eulerAngles(delta).y * kp;
        //  if(turnInput.x == 0) {
        //      turnInput.x = -constructionVelocity.x;
        //      if(abs(turnInput.x) > 1) {
        //          turnInput.x = glm::sign(turnInput.x);
        //      }
        //  }
        //  if(turnInput.y == 0) {
        //      turnInput.y = -constructionVelocity.y;
        //      if(abs(turnInput.y) > 1) {
        //          turnInput.y = glm::sign(turnInput.y);
        //      }
        //  }
        //  if(turnInput.z == 0) {
        //      turnInput.z = (-ridingConstruction->inverseTransformDirection(constructionAngularVelocity).z * kd) - shipRoll * kp;
        //      if(abs(turnInput.z) > 1) {
        //          turnInput.z = glm::sign(turnInput.z);
        //      }
        //  }

        turnControl.z += turnInput.z * 5;

        ridingConstruction->setTurnControl(turnControl);

        position = ridingConstruction->transformPoint(ridingConstructionPoint);

        if (interactInput)
        {
            dismount();
        }
    }

    void doMovement(World *world, float dt)
    {

        auto velocity = body.getVelocity();

        // gravity
        auto gravity = world->getGravityVector(position);
        if (flying)
            gravity = vec3(0);
        if (glm::length(gravity) > 0.05f)
        {
            underGravity = true;
            setUpVector(-gravity);
        }
        else
        {
            underGravity = false;
        }

        // grounded
        if (body.getGroundState() == JPH::CharacterBase::EGroundState::OnGround)
        {
            groundedTimer = coyoteTime;
        }

        // movement
        vec3 relativeVelocity = inverseTransformDirection(velocity);
        vec3 moveXZ = vec3(moveInput.x, 0, moveInput.z);
        if (glm::length(moveXZ) != 0)
            moveXZ = glm::normalize(moveXZ);
        vec3 targetVelocity = moveXZ * moveSpeed;

        if (flying)
        {
            targetVelocity.y = moveInput.y * moveSpeed;
        }
        else
        {
            targetVelocity.y = relativeVelocity.y;
        }

        if (targetedActorID != Invalid_ActorID)
        {
            auto targetedActor = world->getActor<Actor>(targetedActorID);
            if (targetedActor->hasVelocity())
            {
                targetVelocity += inverseTransformDirection(targetedActor->getVelocity());
            }
        }

        if (groundedTimer == 0)
        {
            relativeVelocity = MathHelper::lerp(relativeVelocity, targetVelocity, airAcceleration * dt);
        }
        else
        {
            relativeVelocity = MathHelper::lerp(relativeVelocity, targetVelocity, groundAcceleration * dt);
        }
        velocity = transformDirection(relativeVelocity);

        // jumping
        if (jumpInput > 0 && groundedTimer > 0)
        {
            vec3 relativeVelocity = inverseTransformDirection(velocity);
            relativeVelocity.y = jumpForce;
            velocity = transformDirection(relativeVelocity);
            jumpInput = 0;
            groundedTimer = 0;
        }

        jumpInput = MathHelper::moveTowards(jumpInput, 0, dt);

        // turning in midair
        if (!underGravity)
        {

            angularVelocity.z = MathHelper::lerp(angularVelocity.z, rotationInput * rotationSpeed, rotationAcceleration * dt);
            angularVelocity.x = MathHelper::lerp(angularVelocity.x, fmin(abs(lookPitch), rotationSpeed) * sign(lookPitch), rotationAcceleration * dt);

            lookPitch -= angularVelocity.x * dt;
            vec3 eyePos = getEyePosition();
            rotation = glm::angleAxis(glm::radians(angularVelocity.z) * dt, transformDirection(vec3(0, 0, -1))) * rotation;
            rotation = glm::angleAxis(glm::radians(angularVelocity.x) * dt, transformDirection(vec3(-1, 0, 0))) * rotation;
            position = eyePos - transformDirection(vec3(0, height * 0.5f, 0)); // to rotate around eye
        }

        body.setVelocity(velocity);

        body.update(world, this, position, rotation, gravity, dt);

        // if(!noClip) {
        //     world->collideBasic(this,height,radius);
        // }
    }

    void handleInteraction(World *world)
    {
        if (interactInput)
        {
            interact(world);
        }
    }

    vec3 getItemDropPosition()
    {
        return getEyePosition() + getEyeDirection() * itemDropDistance;
    }

    void handleHeldItem(World *world, float dt)
    {
        auto &selectedStack = toolbar.at(selectedTool);
        if (!selectedStack.isEmpty())
        {
            selectedStack.item->step(world, *this, toolbar.at(selectedTool), dt);
            heldItemData.actionTimer += dt;
            if (dropInput)
            {
                selectedStack.amount--;
                auto droppedItemStack = ItemStack(selectedStack.item, 1, selectedStack.storage);
                world->spawn(ItemActor::makeInstance(droppedItemStack, getEyePosition() + getEyeDirection() * itemDropDistance, getEyeRotation()));
                onHeldItemChanged({this, selectedStack, selectedTool});
            }
        }
    }

    void handleHeldItemClient(World *world, float dt)
    {
        auto &selectedStack = toolbar.at(selectedTool);
        if (!selectedStack.isEmpty())
        {
            selectedStack.item->stepClient(world, *this, toolbar.at(selectedTool), dt);
            heldItemData.actionTimer += dt;
            if (dropInput)
            {
                onItemDropInput({this});
            }
        }
    }

    void receiveToolActionEvent(World *world, Item *tool, int actionEvent)
    {
        auto &selectedStack = toolbar.at(selectedTool);
        if (!selectedStack.isEmpty())
        {
            if (selectedStack.item == tool)
            {
                selectedStack.item->receiveActionEvent(world, *this, selectedStack, actionEvent);
            }
        }
    }

    void attractItems(World *world)
    {
        auto actors = world->overlapSphere(position, 3);
        for (auto actor : actors)
        {
            auto itemActor = dynamic_cast<ItemActor *>(actor);
            if (itemActor != nullptr)
            {
                // should be an acceleration thingy idk
                auto velocity = itemActor->body.getVelocity();
                vec3 delta = (position - itemActor->getPosition());
                velocity = glm::normalize(delta) * 10.0f;
                itemActor->body.setVelocity(velocity);
            }
        }
    }

    void handleCrafting(float dt)
    {
        if (currentRecipe != nullptr)
        {
            if (!craftingStackHasIngredients(*currentRecipe))
            {
                cancelCraft();
            }
            else
            {
                recipeTimer += dt * craftSpeed;
                if (recipeTimer > currentRecipe->time)
                {
                    craftNoCheck(*currentRecipe);
                    recipeTimer = 0;
                }
            }
        }
    }

    void step(World *world, float dt) override
    {

        updateLastTransform();

        if (targetedActorID != Invalid_ActorID)
        {
            vec3 cubePosition = world->getActor<Actor>(targetedActorID)->getPosition();
            quat rotation = glm::quatLookAt(glm::normalize(getEyePosition() - cubePosition), transformDirection(vec3(0.0f, 1.0f, 0.0f)));
            Debug::drawCube(cubePosition, vec3(1.0f), rotation, Color::green, dt);
        }

        if (ridingConstruction != nullptr)
        {

            world->physics_system.GetBodyInterface().SetObjectLayer(body.getCharacter()->GetInnerBodyID(), Layers::DISABLED);

            doConstructionControl(dt);
        }
        else
        {

            world->physics_system.GetBodyInterface().SetObjectLayer(body.getCharacter()->GetInnerBodyID(), Layers::PLAYER);

            doMovement(world, dt);

            // interaction and dropping
            if (!inMenu)
            {
                handleInteraction(world);

                handleHeldItem(world, dt);

                if (targetInput)
                {
                    targetedActorID = Invalid_ActorID;
                    auto hitOpt = world->raycast(getLookRay(), std::numeric_limits<float>::max(), LayerMask::excludes({Layers::PLAYER, Layers::ITEM}));
                    if (hitOpt)
                    {
                        auto hit = hitOpt.value();
                        if (hit.actor != nullptr)
                        {
                            targetedActorID = hit.actor->id;
                        }
                    }
                }
            }

            attractItems(world);
        }

        // reset inputs
        targetInput = false;
        clickInput = false;
        interactInput = false;
        dropInput = false;
        heldItemData.scrollDelta = 0;
        heldItemData.mouseDelta = vec2(0.0f);

        handleCamera(world);

        handleCrafting(dt);
    }

    void stepClient(World *world, float dt)
    {
        updateLastTransform();

        if (ridingConstruction != nullptr)
        {

            doConstructionControl(dt);
        }
        else
        {

            doMovement(world, dt);

            handleHeldItemClient(world, dt);
        }

        handleCamera(world);

        // reset inputs
        clickInput = false;
        interactInput = false;
        dropInput = false;
        heldItemData.scrollDelta = 0;
    }

    void destroy(World *world) override
    {
        Actor::destroy(world);
        body.destroy(world);
    }

    void ride(Construction *construction, ivec3 point, quat rotation)
    {
        if (ridingConstruction != nullptr)
        {
            dismount();
        }
        // body->
        ridingConstruction = construction;
        ridingConstructionPoint = point;
        ridingConstructionRotation = rotation;
        this->rotation = construction->getRotation() * ridingConstructionRotation * glm::quat(glm::radians(vec3(0.0f, 180.0f, 0.0f)));
        lookPitch = 0;
        turnInput = {};
    }

    void dismount()
    {
        position += ridingConstruction->transformDirection(vec3(0, 1, 0));
        ridingConstruction->setMoveControl(vec3(0));
        ridingConstruction->setTurnControl(vec3(0));
        ridingConstruction = nullptr;
    }

    void interact(World *world)
    {
        if (ridingConstruction != nullptr)
        {
            dismount();
            return;
        }
        auto hitOpt = world->raycast(Ray(getEyePosition(), getEyeDirection()), 10, LayerMask::excludes({Layers::PLAYER, Layers::ITEM}));
        if (hitOpt)
        {
            auto hit = hitOpt.value();
            Construction *construction = dynamic_cast<Construction *>(hit.actor);
            if (construction != nullptr)
            {
                std::cout << "interacted with construction" << std::endl;
                vec3 interactPointWorld = hit.point - hit.normal * 0.5f;
                vec3 interactPointLocal = construction->inverseTransformPoint(interactPointWorld);
                ivec3 interactPointInt = glm::round(interactPointLocal);
                auto data = construction->getBlock(interactPointInt);
                auto block = data.block;
                auto storage = data.storage;
                if (data.block != nullptr)
                {
                    block->onInteract(construction, interactPointInt, storage, *this);
                }
            }
        }
    }

    // could later be abstracted to a controller
    void processInput(Input &input)
    {

        // eventually we want an enum for keys instead of using the defines

        if (inMenu)
        {
            processInputInventory(input);
        }
        else
        {
            processInputNormal(input);
        }
    }

    void processInputNormal(Input &input)
    {
        moveInput = vec3(0, 0, 0);
        // turnInput = vec3(0,0,0);
        if (input.getKey(GLFW_KEY_W))
        {
            moveInput.z -= 1; // im not sure why this exists :shrug:
        }
        if (input.getKey(GLFW_KEY_S))
        {
            moveInput.z += 1;
        }
        if (input.getKey(GLFW_KEY_A))
        {
            moveInput.x -= 1;
        }
        if (input.getKey(GLFW_KEY_D))
        {
            moveInput.x += 1;
        }
        if (input.getKey(GLFW_KEY_SPACE))
        {
            moveInput.y += 1;
        }
        if (input.getKey(GLFW_KEY_C))
        {
            moveInput.y -= 1;
        }
        rotationInput = 0;
        turnInput.z = 0;
        if (input.getKey(GLFW_KEY_Q))
        {
            rotationInput -= 1;
            turnInput.z -= 1;
        }
        if (input.getKey(GLFW_KEY_E))
        {
            rotationInput += 1;
            turnInput.z += 1;
        }

        if (input.getKeyPressed(GLFW_KEY_SPACE))
        {
            jumpInput = inputBuffer;
        }

        // probably temp for
        if (input.getKey(GLFW_KEY_UP))
        {
            turnInput.x -= 1; // im not sure why this exists :shrug:
        }
        if (input.getKey(GLFW_KEY_DOWN))
        {
            turnInput.x += 1;
        }
        if (input.getKey(GLFW_KEY_LEFT))
        {
            turnInput.y += 1;
        }
        if (input.getKey(GLFW_KEY_RIGHT))
        {
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

        if (input.getKeyPressed(GLFW_KEY_F))
        {
            interactInput = true;
        }

        if (input.getKeyPressed(GLFW_KEY_G))
        {
            dropInput = true;
        }

        brakeInput = input.getKey(GLFW_KEY_LEFT_SHIFT) || input.getKey(GLFW_KEY_RIGHT_SHIFT);

        if (input.getKeyPressed(GLFW_KEY_1))
        {
            setCurrentTool(0);
        }
        if (input.getKeyPressed(GLFW_KEY_2))
        {
            setCurrentTool(1);
        }
        if (input.getKeyPressed(GLFW_KEY_3))
        {
            setCurrentTool(2);
        }
        if (input.getKeyPressed(GLFW_KEY_4))
        {
            setCurrentTool(3);
        }
        if (input.getKeyPressed(GLFW_KEY_5))
        {
            setCurrentTool(4);
        }
        if (input.getKeyPressed(GLFW_KEY_6))
        {
            setCurrentTool(5);
        }
        if (input.getKeyPressed(GLFW_KEY_7))
        {
            setCurrentTool(6);
        }
        if (input.getKeyPressed(GLFW_KEY_8))
        {
            setCurrentTool(7);
        }
        if (input.getKeyPressed(GLFW_KEY_9))
        {
            setCurrentTool(8);
        }
        if (input.getKeyPressed(GLFW_KEY_Z))
        {
            thirdPerson = !thirdPerson;
        }
        if (input.getKeyPressed(GLFW_KEY_T))
        {
            shake.startShake();
        }

        // check for cheats access
        if (input.getKeyPressed(GLFW_KEY_F2))
        {
            noClip = !noClip;
        }

        if (input.getKeyPressed(GLFW_KEY_X))
        {
            flying = !flying;
        }

        if (input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_3))
        {
            targetInput = true;
        }

        // if(!toolbar[selectedTool].isEmpty()) {
        //     toolbar[selectedTool].item->processInput(input);
        // }
        heldItemData.scrollDelta += input.currentMouseScrollDelta;
        if (input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_1))
        {
            heldItemData.clickInput = true;
        }
        if (input.getMouseButtonPressed(GLFW_MOUSE_BUTTON_2))
        {
            heldItemData.altClickInput = true;
        }
        heldItemData.clickHold = input.getMouseButton(GLFW_MOUSE_BUTTON_1);
        heldItemData.altClickHold = input.getMouseButton(GLFW_MOUSE_BUTTON_2);

        if (input.getKeyPressed(GLFW_KEY_TAB) && !inventoryDisabled)
        {
            openMenu();
        }

        auto delta = input.getMouseDelta();
        heldItemData.mouseDelta += delta;

        turnInput.x += delta.y * 0.01f;
        turnInput.y -= delta.x * 0.01f;
        // std::cout << StringHelper::toString(delta) << std::endl;
        moveMouse(delta * 0.01f);
    }

    void processInputInventory(Input &input)
    {
        moveInput = vec3(0, 0, 0);
        if (input.getKeyPressed(GLFW_KEY_TAB))
        {
            closeMenu();
        }
        if (input.getKeyPressed(GLFW_KEY_F))
        {
            closeMenu();
        }
        input.getMouseDelta();
    }

    void setCurrentTool(int index)
    {

        if (index == selectedTool)
            return; // dont do anything if its the same tool

        if (!toolbar[selectedTool].isEmpty())
        {
            toolbar[selectedTool].item->unequip(*this);
        }
        selectedTool = index;
        if (!toolbar[selectedTool].isEmpty())
        {
            toolbar[selectedTool].item->equip(*this);
            heldItemData.setAction(0); // reset actions
            heldItemData.clickInput = false;
            heldItemData.clickHold = false;
        }

        onHeldItemChanged({this, toolbar[selectedTool], selectedTool});
    }

    void setToolbar(int index, ItemStack stack)
    {
        toolbar[index] = stack;
    }

    // for tools to do when they reduce count etc
    void refreshTool()
    {
        setCurrentTool(selectedTool);
    }

    void closeMenu()
    {
        inMenu = false;
        openMenuObject = nullptr;

        returnCursor();
    }

    void openMenu()
    {
        openMenu(nullptr);
    }

    void openMenu(std::unique_ptr<MenuObject> menuObject)
    {
        heldItemData.setAction(0);
        inMenu = true;
        openMenuObject = std::move(menuObject);
    }

    // we should turn this into a crafter module. tho it needs to be able to handle variables in storage too idk
    void startCraft(Recipe &recipe)
    {
        if (!hasIngredients(recipe))
        {
            return; // dont start the craft
        }
        if (recipe.ingredients.size() != 1)
        {
            Debug::warn("player recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
            return;
        }
        // should have them so we dont need to check

        if (craftingStack.tryInsert(recipe.ingredients[0]))
        {
            take(recipe.ingredients[0]);
        }
        else
        {
            // try to take the item out
            if (!craftingStack.isEmpty())
            {
                give(craftingStack);
                craftingStack.clear();
                // try again
                if (craftingStack.tryInsert(recipe.ingredients[0]))
                {
                    take(recipe.ingredients[0]);
                }
            }
        }
        if (currentRecipe != &recipe)
        {
            currentRecipe = &recipe;
            recipeTimer = 0;
        }
    }

    void cancelCraft()
    {
        currentRecipe = nullptr;
        recipeTimer = 0;
    }

    void itemSlotHoverActions(DrawContext context, ItemStack &stack, ItemSlotInteractOptions options = {})
    {
        if (!options.allowInsert && !cursorStack.isEmpty())
        {
            return; // block insertion
        }
        if (!options.allowRemove && !stack.isEmpty())
        {
            return; // block removal
        }
        if (context.mouseLeftClicked())
        {
            stack = replaceCursor(stack);
            return;
        }
        // insert or take one TODO: IDK what to do lol
        if (context.mouseRightClicked())
        {
            if (!cursorStack.isEmpty() && stack.canInsert(cursorStack))
            {
                // place one
                cursorStack.amount--;
                stack.tryInsert(ItemStack(cursorStack.item, 1, cursorStack.storage));
                return;
            }
            if (!stack.isEmpty() && cursorStack.canInsert(stack))
            {
                stack.amount--;
                cursorStack.tryInsert(ItemStack(stack.item, 1, stack.storage));
                return;
            }
        }
    }

    // returns the itemstack in cursor
    ItemStack replaceCursor(ItemStack stack)
    {
        if (stack.tryInsert(cursorStack))
        {
            // insert
            cursorStack.clear();
            return stack;
        }
        else
        {
            // swap em
            ItemStack returnStack = cursorStack;
            cursorStack = stack;
            return returnStack;
        }
    }

    // when theres no item to replace
    ItemStack dropCursor()
    {
        ItemStack returnStack = cursorStack;
        cursorStack.clear();
        return returnStack;
    }

    void give(ItemStack stack)
    {
        onInventoryChanged({this, stack});
        for (auto &toolbarStack : toolbar)
        {
            if (toolbarStack.tryInsert(stack))
            {
                return;
            }
        }
        inventory.give(stack);
    }

    int take(ItemStack stack)
    {
        return take(stack.item, stack.amount);
    }

    int take(Item *item, int amount)
    {
        onInventoryChanged({this, ItemStack(item, amount), true});
        int amountTaken = 0;
        for (auto &toolbarStack : toolbar)
        {
            amountTaken += toolbarStack.take(ItemStack(item, amount));
            if (amountTaken >= amount)
            {
                return amount;
            }
        }
        return inventory.take(ItemStack(item, amount - amountTaken)) + amountTaken;
    }

    // this is kinda janky :shrug:
    bool hasIngredients(Recipe &recipe)
    {
        for (auto &ingredient : recipe.ingredients)
        {
            // basically reduce the required by invenyory
            int requiredByInventory = ingredient.amount;
            for (auto toolbarStack : toolbar)
            {
                if (toolbarStack.item == ingredient.item)
                {
                    if (toolbarStack.amount > ingredient.amount)
                    {
                        requiredByInventory = 0;
                        break; // dont even need to check inventory
                    }
                    else
                    {
                        requiredByInventory -= toolbarStack.amount;
                    }
                }
            }
            if (requiredByInventory > 0 && !inventory.has(ItemStack(ingredient.item, requiredByInventory)))
            {
                return false;
            }
        }
        return true;
    }

    bool craftingStackHasIngredients(Recipe &recipe)
    {
        if (recipe.ingredients.size() != 1)
        {
            Debug::warn("player recipe has " + std::to_string(recipe.ingredients.size()) + " ingredients (must be 1)");
            return false;
        }
        if (!craftingStack.has(recipe.ingredients[0]))
            return false;
        return true;
    }

    bool tryCraft(Recipe &recipe)
    {
        if (!craftingStackHasIngredients(recipe))
            return false;
        craftNoCheck(recipe);

        return true;
    }

    void craftNoCheck(Recipe &recipe)
    {
        craftingStack.take(recipe.ingredients[0]);
        give(recipe.result);
    }

    void returnCursor()
    {
        give(cursorStack);
        cursorStack.clear();
    }

    void moveMouse(vec2 delta)
    {
        rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity, transformDirection(vec3(0, -1, 0))) * rotation;
        lookPitch += delta.y * lookSensitivity;
        if (lookPitch > 89.9f)
            lookPitch = 89.9f;
        if (lookPitch < -89.9f)
            lookPitch = -89.9f;
    }

    void setCamera(Camera &camera, float interpolation)
    {
        camera.position = MathHelper::lerp(lastCameraPosition, currentCameraPosition, interpolation);
        camera.rotation = glm::slerp(lastCameraRotation, currentCameraRotation, interpolation) * shake.getRotation();
    }

    static std::unique_ptr<Character> makeDefaultPrototype()
    {
        auto ptr = new Character();
        return std::unique_ptr<Character>(ptr);
    }

    static std::unique_ptr<Character> makeInstance(Character *prototype, vec3 position = vec3(0), quat rotation = glm::identity<quat>())
    {
        return makeInstanceFromPrototype<Character>(prototype, position, rotation);
    }

    glm::mat4 getTransform()
    {
        return Actor::getTransform();
    }

    vec3 getEyePosition()
    {
        return transformPoint(vec3(0, height / 2.0f, 0));
    }

    vec3 getEyePositionInterpolated(float interpolation)
    {
        return transformPointInterpolated(vec3(0, height / 2.0f, 0), interpolation);
    }

    quat getEyeRotation()
    {
        return rotation * glm::angleAxis(glm::radians(lookPitch), vec3(-1, 0, 0));
    }

    vec3 getEyeDirection()
    {
        return getEyeRotation() * vec3(0, 0, -1);
    }

    Ray getLookRay()
    {
        return Ray(getEyePosition(), getEyeDirection());
    }

    bool playerStep()
    {
        return true;
    }

    string getTypeName()
    {
        return "character";
    }

    string getActorDataType()
    {
        return getTypeName();
    }

    virtual std::vector<std::uint8_t> createSaveBuffer()
    {
        auto data = save();
        auto buf = cista::serialize(data);
        return buf;
    }

    data_Character save()
    {
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

    void load(const data_Character &data, DataLoader &loader)
    {
        Actor::load(data.actor);
        body.load(data.body);
        lookPitch = data.lookPitch;
        selectedTool = data.selectedTool;
        refreshTool();
        for (size_t i = 0; i < data.toolbar.size() && i < toolbarSize; i++)
        {
            toolbar[i].load(data.toolbar[i], loader);
        }
        inventory.load(data.inventory, loader);
        thirdPerson = data.thirdPerson;
    }

    static std::unique_ptr<Actor> makeInstanceFromSave(data_Character &data, Character *prototype, DataLoader &loader)
    {
        if (prototype == nullptr)
            throw std::runtime_error("prototype is null");
        auto actor = makeInstanceFromPrototype(prototype);
        actor->load(data, loader);

        std::cout << "LOADING PLAYER ACTOR" << std::endl;

        return actor;
    }

protected:
    Character() : Actor()
    {
    }
};