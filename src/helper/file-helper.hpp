#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using std::string, std::ifstream, std::ofstream, std::getline, std::vector;

namespace FileHelper {

    std::vector<char> readBinary(string path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary); //ate is for starting at the end

        if (!file.is_open()) {
            std::cout << "[WARNING] failed to open file at path " << path << std::endl;
            if (file.bad()) {
                std::cout << "Fatal error: badbit is set." << std::endl;
            }

            if (file.fail()) {
                // Print a more detailed error message using
                // strerror
                std::cout << "Error details: " << strerror(errno)
                    << std::endl;
            }
            return std::vector<char>(0);
        }

        size_t fileSize = (size_t) file.tellg(); //because we started at the end, we can get the file size
        std::vector<char> buffer(fileSize);
        
        file.seekg(0); //go back to the start
        file.read(buffer.data(), fileSize); //read as normal

        file.close();

        return buffer;

    }

    string readToString(string path) {

        string text;
        ifstream file(path);
        if(!file.good()) {
            std::cout << "[WARNING] no file at path " << path << std::endl;
            return "";
        }

        string line;
        while (getline (file, line)) {
            text += line + "\n";
        }

        file.close(); 
        return text;
    }

    vector<string> readToStrings(string path) {

        vector<string> text;
        ifstream file(path);
        if(!file.good()) {
            std::cout << "[ERROR] no file at path " << path << std::endl;
            return text;
        }

        string line;
        while (getline (file, line)) {
            text.push_back(line);
        }

        file.close(); 
        return text;
    }
};