//
// Created by cilan on 7/6/2017.
//

#include "Texture.h"
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <iostream>

Texture::Texture() {
    // do nothing
}

Texture::Texture(GLenum textureTarget, const std::string& texturePath) : m_texturePath(texturePath),
                                                                         m_textureTarget(textureTarget) {
    // do nothing
}

bool Texture::load() {
    if (!imageData.loadFromFile(m_texturePath)) {
        std::cout << "Could not load " << m_texturePath << std::endl;
    }

    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureTarget, m_textureObj);
    glTexImage2D(
            m_textureTarget, 0, GL_RGBA,
            imageData.getSize().x, imageData.getSize().y,
            0,
            GL_RGBA, GL_UNSIGNED_BYTE, imageData.getPixelsPtr()
    );

    glGenerateMipmap(m_textureTarget);
    glTexParameteri(m_textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(m_textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return true;
}

void Texture::bind(GLenum textureUnit) {
    glActiveTexture(textureUnit);
    glBindTexture(m_textureTarget, m_textureObj);
}
