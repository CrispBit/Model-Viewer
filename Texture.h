//
// Created by cilan on 7/6/2017.
//

#ifndef TEXTURE_H
#define	TEXTURE_H

#include <string>
#include <SFML/Graphics.hpp>

#include <GL/glew.h>

class Texture {
public:
    Texture(GLenum textureTarget, const std::string& fileName);
    bool load();
    void bind(GLenum TextureUnit);
private:
    std::string m_texturePath;
    GLenum m_textureTarget;
    GLuint m_textureObj;
    sf::Image imageData;
};

#endif
