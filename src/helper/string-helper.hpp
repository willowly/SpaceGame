#pragma once
#include <vector>
#include <string>
#include <format>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

using std::string, glm::vec4, glm::vec3, glm::vec2,glm::ivec2,glm::ivec3,glm::mat4,glm::quat;

namespace StringHelper {

    inline std::vector<std::string> split(std::string s, const std::string& delimiter,int limit = 0) {
        std::vector<std::string> tokens;
        size_t pos = 0;
        std::string token;
        int i = 0;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            tokens.push_back(token);
            s.erase(0, pos + delimiter.length());
            i++;
            if(limit != 0 && i >= limit) {
                break;
            }
        }
        tokens.push_back(s);

        return tokens;
    }

    inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    inline std::string toString(vec4 v) {
        return std::format("<{:5.3f},{:5.3f},{:5.3f},{:5.3f}>",v.x,v.y,v.z,v.w);
    }
    inline std::string toString(quat v) {
        return std::format("<{:5.3f},{:5.3f},{:5.3f},{:5.3f}>",v.x,v.y,v.z,v.w);
    }
    inline std::string toString(vec3 v) {
        return std::format("<{:5.3f},{:5.3f},{:5.3f}>",v.x,v.y,v.z);
    }
    inline std::string toString(ivec3 v) {
        return std::format("<{},{},{}>",v.x,v.y,v.z);
    }
    inline std::string toString(vec2 v) {
        return std::format("<{:5.3f},{:5.3f}>",v.x,v.y);
    }
    inline std::string toString(ivec2 v) {
        return std::format("<{},{}>",v.x,v.y);
    }

    inline std::string toString(mat4 v) {
        string s = "[";
        for (size_t j = 0; j < 4; j++)
        {
            for (size_t i = 0; i < 4; i++)
            {
                s += std::to_string(v[i][j]) + ",";
            }
            s += "\n";
        }
        s += "]";
        return s;
        
    }
}