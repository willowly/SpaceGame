#pragma once

#include <glm/glm.hpp>
#include <map>

using glm::vec2, std::map, std::string;

// tracks the current inputs 
class Input {


    public:
        vec2 lastMousePosition = {};
        vec2 currentMousePosition = {};
        vec2 mouseDelta = {};
        map<int,bool> keys;
        map<int,bool> keysPressed;
        map<int,bool> keysReleased;
        map<int,bool> mouseButtons;
        map<int,bool> mouseButtonsPressed;
        map<int,bool> mouseButtonsReleased;

        string textInput;

        bool getKey(int scanCode) {
            if(keys.contains(scanCode)) {
                return keys.at(scanCode);
            } else {
                return false;
            }
        }

        bool getKeyPressed(int scanCode) {
            if(keysPressed.contains(scanCode)) {
                return keys.at(scanCode);
            } else {
                return false;
            }
        }

        bool getKeyReleased(int scanCode) {
            if(keysReleased.contains(scanCode)) {
                return keys.at(scanCode);
            } else {
                return false;
            }
        }

        bool getMouseButton(int scanCode) {
            if(mouseButtons.contains(scanCode)) {
                return mouseButtons.at(scanCode);
            } else {
                return false;
            }
        }

        bool getMouseButtonPressed(int scanCode) {
            if(mouseButtonsPressed.contains(scanCode)) {
                return mouseButtonsPressed.at(scanCode);
            } else {
                return false;
            }
        }

        bool getMouseButtonReleased(int scanCode) {
            if(mouseButtonsReleased.contains(scanCode)) {
                return mouseButtonsReleased.at(scanCode);
            } else {
                return false;
            }
        }

        vec2 getMouseDelta() {
            return mouseDelta;
        }

        string getTextInput() {
            string textToSend = textInput;
            textInput = "";
            return textToSend;
        }

        void clearInputBuffers() {
            textInput = "";
            keysPressed.clear();
            keysReleased.clear();
            mouseButtonsPressed.clear();
            mouseButtonsReleased.clear();
            mouseDelta = currentMousePosition - lastMousePosition;
            lastMousePosition = currentMousePosition;
        }

        
};