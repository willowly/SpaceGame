#pragma once
#include <vector>
#include <string>
#include <format>

#include <include/glm/glm.hpp>

using glm::vec3, glm::vec2;

namespace StringHelper {

    std::vector<std::string> split(std::string s, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        tokens.push_back(s);

        return tokens;
    }

    std::string toString(vec3 v) {
        return std::format("<{},{},{}>",v.x,v.y,v.z);
    }
    std::string toString(vec2 v) {
        return std::format("<{},{}>",v.x,v.y);
    }
}