#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using std::string, std::ifstream, std::ofstream, std::getline, std::vector;

namespace FileHelper {
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