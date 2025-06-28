#pragma once

#include <graphics/text.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/input.hpp>
#include <engine/debug.hpp>

#include <sol/sol.hpp>

class Console {

    public:

        Console(string fontPath) : inputText(fontPath,20) {
            
            
        }

        bool enabled;
        string inputString;
        Text inputText;
        string response;
        // sf::Text responseText;

        vector<string> history;
        int historyIndex;

        void render(glm::vec2 windowSize,Shader& shader) {

            inputText.text = "> " + inputString;
            inputText.position = glm::vec2(10,windowSize.y - 20);
            inputText.render(shader,glm::ortho(0.0f,windowSize.x,0.0f,windowSize.y));

        }

        void processInput(Input& input,sol::state& lua) {
            if(input.getKeyPressed(GLFW_KEY_BACKSPACE)) {
                if(inputString.size() > 0) {
                    inputString = inputString.substr(0,inputString.size()-1);
                }
            }
            if(input.getKeyPressed(GLFW_KEY_ENTER)) {
                

                sol::load_result load_result = lua.load("return " + inputString);

                if(!load_result.valid()) {
                    load_result = lua.load(inputString);
                } 
                
                if(!load_result.valid()) {
                    sol::error e = load_result;
                    Debug::warn("Console load error: " + (std::string)e.what());
                } else {
                    sol::function func = load_result;
                    auto result = func();
                    if(result.valid()) {
                        for(sol::object obj : result) {
                            std::string returnString = lua["tostring"](obj);
                            Debug::info("Console return: " + returnString,InfoPriority::HIGH);
                        }
                    } else {
                        sol::error e = result;
                        Debug::warn("Console error: " + (std::string)e.what());
                    }
                }

                historyIndex = 0;
                history.insert(history.begin(),inputString);
                inputString = "";
                return;
            }
            
            inputString += input.getTextInput();
            inputString.erase(std::remove(inputString.begin(), inputString.end(), '`'), inputString.end());
        }

        // void getEvent(sf::Event event) {
        //     if(event.type == sf::Event::KeyPressed) {
        //         if(event.key.code == sf::Keyboard::Up) {
        //             if(historyIndex < history.size()) {
        //                 input = history[historyIndex];
        //                 historyIndex++;
        //             }
        //         }
        //         if(event.key.code == sf::Keyboard::Down) {
        //             if(historyIndex > 0) {
        //                 historyIndex--;
        //                 input = history[historyIndex];
        //             }
        //         }
        //     }
        //     if(event.type == sf::Event::TextEntered) {

        //         if(event.text.unicode == 96) return; //code for tilde, dont use it in the text
        //         if(event.text.unicode == 8) { //code for backspace
        //             if(input.size() > 0) {
        //                 input = input.substr(0,input.size()-1);
        //             }
        //             return;
        //         }
        //         if(event.text.unicode == 10) { //return key
        //             Lua::log += "> " + input + "\n";

        //             sol::load_result load_result = Lua::lua.load("return " + input);

        //             if(!load_result.valid()) {
        //                 load_result = Lua::lua.load(input);
        //             } 
                    
        //             if(!load_result.valid()) {
        //                 sol::error e = load_result;
        //                 Lua::log += "[LOAD ERROR] " + (std::string)e.what() + "\n";
        //                 std::cout << "[LOAD ERROR] " << (std::string)e.what() << std::endl;
        //             } else {
        //                 sol::function func = load_result;
        //                 auto result = func();
        //                 if(result.valid()) {
        //                     for(sol::object obj : result) {
        //                         std::string returnString = Lua::lua["tostring"](obj);
        //                         Lua::log += returnString + "\n";
        //                     }
        //                 } else {
        //                     sol::error e = result;
        //                     Lua::log += "[ERROR] " + (std::string)e.what() + "\n";
        //                     std::cout << "[ERROR] " << (std::string)e.what() << std::endl;
        //                 }
        //             }

        //             historyIndex = 0;
        //             history.insert(history.begin(),input);
        //             input = "";
        //             return;
        //         }
        //         input += event.text.unicode;
                
        //     }
        // }

};