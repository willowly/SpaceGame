#pragma once

#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "color.hpp"

#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_STROKER_H

using std::string;

class Text {
    public:
        
        static void loadFreeType(FT_Library ft) {
            if (FT_Init_FreeType(&ft))
            {
                std::cout << "[ERROR] FREETYPE: Could not init FreeType Library" << std::endl;
            }
        }

        struct Character {
            unsigned int textureID;  // texture
            glm::ivec2   Size;       // Size of glyph
            glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
            unsigned int Advance;    // Offset to advance to next glyph
        };

        std::map<char, Character> Characters;

        unsigned int VBO = -1;
        unsigned int VAO = -1;
        glm::vec2 position = glm::vec2(1.0f);
        Color color = Color::white;
        string text = "";
        glm::vec2 scale = glm::vec2(1.0f);

        

        Text(string fontPath,int characterSize) {

            FT_Library ft;
            if (FT_Init_FreeType(&ft))
            {
                std::cout << "[ERROR] FREETYPE: Could not init FreeType Library" << std::endl;
            }
            FT_Face face;
            if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
            {
                std::cout << "[ERROR] FREETYPE: Failed to load font" << std::endl;  
            }
            FT_Set_Pixel_Sizes(face, 0, characterSize);


            glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // because the texture will not be a multiple of 4 with single byte colors
            
            //load all the characters
            for (unsigned char c = 0; c < 128; c++)
            {
                // load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER))
                {
                    std::cout << "[ERROR] FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                // generate texture
                
                unsigned int texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    face->glyph->bitmap.width,
                    face->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    face->glyph->bitmap.buffer
                );
                // set texture options 
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                // now store character for later use
                Character character = {
                    texture, 
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    (unsigned int)face->glyph->advance.x
                };
                Characters.insert(std::pair<char, Character>(c, character));
            }

            FT_Done_Face(face);
            FT_Done_FreeType(ft);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            // text rendering
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

        }
        void render(Shader& shader,glm::mat4 projection) {

            glDisable(GL_DEPTH_TEST);
            //glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
            
            shader.use();
            shader.setColor3("color",color);
            shader.setMat4("projection",projection);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(VAO);

            int x = position.x;
            int y = position.y;
            for (char c : text)
            {
                Character ch = Characters[c];

                float xpos = x + ch.Bearing.x * scale.x;
                float ypos = y - (ch.Size.y - ch.Bearing.y) * scale.y;

                float w = ch.Size.x * scale.x;
                float h = ch.Size.y * scale.y;
                // update VBO for each character
                float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },            
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }           
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.textureID);
                    //update content of VBO memory
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                // render quad
                 glDrawArrays(GL_TRIANGLES, 0, 6);
                // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                x += (ch.Advance >> 6) * scale.x; // bitshift by 6 to get value in pixels (2^6 = 64)
            }

            glEnable(GL_DEPTH_TEST);
            
            
        }
};