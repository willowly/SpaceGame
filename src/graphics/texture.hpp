#pragma once
#include <vector>
#include <string>
#include "stb_image.h"
#include <iostream>

using std::string;

class Texture {

    
    unsigned int textureID;

    public:
        enum Format {
            RGB,
            RGBA
        };
        Texture() {
            glGenTextures(1,&textureID);
            //std::cout << "new texture id: " << textureID << std::endl;
        }
        ~Texture() {
            glDeleteTextures(1,&textureID);
            //std::cout << "deleting texture id: " << textureID << std::endl;
        }
        Texture (Texture&& obj) : textureID(obj.textureID) {
            obj.textureID = -1;
        }
        void loadFromFile(string path,Format format) {

            // bind it
            glBindTexture(GL_TEXTURE_2D,textureID);

            // set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            // load the image
            int width, height, nrChannels;
            stbi_set_flip_vertically_on_load(true); 
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            if(data) {
                auto glFormat = GL_RGBA;
                switch(format) {
                    case RGB:
                        glFormat = GL_RGB;
                    case RGBA:
                        glFormat = GL_RGBA;
                }
                // load the data to the gpu
                glTexImage2D(GL_TEXTURE_2D,0,glFormat,width,height,0,glFormat,GL_UNSIGNED_BYTE,data);
                glGenerateMipmap(GL_TEXTURE_2D);
            } else {
                std::cout << "[WARNING] could not load texture " + path << std::endl;
            }

            stbi_image_free(data); //get rid of the data
        }

        void bind() {
            glBindTexture(GL_TEXTURE_2D,textureID);
        }

        void setLinearFiltering(){
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        void setPointFiltering(){
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        int getID() {
            return textureID;
        }
};