#pragma once
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
    
};

Color Color::red = Color(1,0,0);
Color Color::green = Color(1,0,0);
Color Color::blue = Color(1,0,0);

Color Color::magenta = Color(1,0,1);
Color Color::cyan = Color(0,1,1);
Color Color::yellow = Color(1,1,0);

Color Color::white = Color(1,1,1);
Color Color::black = Color(0,0,0);
Color Color::clear = Color(0,0,0,0);
