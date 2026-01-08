#pragma once
#include "glm/glm.hpp"

using glm::vec3, glm::vec4;


class Color {

    
    public:
        Color(float r,float g, float b, float a){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        Color(float r,float g, float b){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = 1;
        }
        Color(vec3 vec){
            this->r = vec.x;
            this->g = vec.y;
            this->b = vec.z;
            this->a = 1;
        }
        Color(vec4 vec){
            this->r = vec.x;
            this->g = vec.y;
            this->b = vec.z;
            this->a = vec.w;
        }
        Color(){
        }
        float r;
        float g;
        float b;
        float a;

        static Color red;
        static Color green;
        static Color blue;

        static Color magenta;
        static Color cyan;
        static Color yellow;

        static Color white;
        static Color black;
        static Color clear;

        vec3 asVec3() {
            return vec3(r,g,b);
        }

        vec4 asVec4() {
            return vec4(r,g,b,a);
        }
    
};

Color Color::red = Color(1,0,0);
Color Color::green = Color(0,1,0);
Color Color::blue = Color(0,0,1);

Color Color::magenta = Color(1,0,1);
Color Color::cyan = Color(0,1,1);
Color Color::yellow = Color(1,1,0);

Color Color::white = Color(1,1,1);
Color Color::black = Color(0,0,0);
Color Color::clear = Color(0,0,0,0);
