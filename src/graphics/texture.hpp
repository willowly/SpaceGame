#pragma once
#include <vector>
#include <string>
#include <OpenGL/gl3.h>
#include "include/stb_image.h"
#include <iostream>

using std::string;

class Texture {


    unsigned int textureID;

    public:
        Texture() {
            glGenTextures(1,&textureID);
        }
        ~Texture() {
            glDeleteTextures(1,&textureID);
        }
        void loadFromFile(string path) {

            // bind it
            glBindTexture(GL_TEXTURE_2D,textureID);

            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // load the image
            int width, height, nrChannels;
            stbi_set_flip_vertically_on_load(true); 
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            if(data) {

                // load the data to the gpu
                glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,data);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                std::cout << "[ERROR] could not load texture " + path << std::endl;
            }

            stbi_image_free(data); //get rid of the data
        }

        void bind() {
            glBindTexture(GL_TEXTURE_2D,textureID);
        }
};