#pragma once
#include "actor.hpp"
#include <include/glm/glm.hpp>
#include <include/glm/gtc/quaternion.hpp>
#include <include/glm/gtx/quaternion.hpp>
#include <format>
#include <string>
#include <iostream>
#include "helper/math-helper.hpp"
#include "helper/string-helper.hpp"

class Character : public Actor {
    public: 

        Character() {

        }
        Character(Model* model,Material* material) : Actor(model,material) {

        }
        float moveSpeed = 5.0f;
        float lookPitch = 0;
        float lookSensitivity = 5;
        float height = 0.75;
        float acceleration = 10;
        float jumpForce = 10;

        void step(float dt) {

            float moveSpeed = 5.0f;
            vec3 moveVector = vec3(0);
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
                moveVector += vec3(0,0,-1);
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
                moveVector += vec3(0,0,1);
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) {
                moveVector += vec3(-1,0,0);
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) {
                moveVector += vec3(1,0,0);
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                if(position.y <= 0) {
                    velocity.y = jumpForce;
                }
            }
            // if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
            //     moveVector += vec3(0,-1,0);
            // }
            
            vec3 targetVelocity = glm::inverse(rotation) * moveVector * moveSpeed;
            vec3 moveVelocity = vec3(velocity.x,0,velocity.z);
            moveVelocity = MathHelper::lerp(moveVelocity,targetVelocity,acceleration*dt);

            velocity.x = moveVelocity.x;
            velocity.z = moveVelocity.z;

            Actor::step(dt);

            // floor collision
            if(position.y < 0) {
                position.y = 0;
                velocity.y = 0;
            }
            //std::cout << std::format("<{},{},{}>",position.x,position.y,position.z) << std::endl;

        }

        void moveMouse(vec2 delta) {
            rotation = glm::angleAxis(glm::radians(delta.x) * lookSensitivity,vec3(0,1,0)) * rotation;
            lookPitch += delta.y * lookSensitivity;
            if(lookPitch > 89.9f) lookPitch = 89.9f;
            if(lookPitch < -89.9f) lookPitch = -89.9f;
        }

        void firstPersonCamera(Camera& camera) {
            camera.position = position + vec3(0,height,0);
            camera.rotation = glm::angleAxis(glm::radians(lookPitch),vec3(1,0,0)) * rotation;
        }

        bool playerStep() {
            return true;
        }

};