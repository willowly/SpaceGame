#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>

using std::string, std::ifstream, std::ofstream, std::getline, std::vector;

namespace FileHelper {

    std::vector<char> readBinary(string path);

    void writeBinary(string path,std::vector<std::uint8_t> buffer);

    string readToString(string path);

    vector<string> readToStrings(string path);

}