#pragma once
#include "graphics/model.hpp"
#include "graphics/texture.hpp"
#include "graphics/shader.hpp"
#include <map>

using std::map;

class Registry {
    
    public:
        map<string,Model> models;
        map<string,Texture> textures;
        map<string,Material> materials;

        Shader litShader;
        Shader textShader;

};