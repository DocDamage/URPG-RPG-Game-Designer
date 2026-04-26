#include "gl_texture.h"
#include <SDL2/SDL_opengl.h>
#include <iostream>

namespace urpg {

Texture::Texture() {}

Texture::~Texture() {
    if (m_textureId != 0) {
        glDeleteTextures(1, &m_textureId);
    }
}

bool Texture::loadFromMemory(const std::vector<uint8_t>& pixelData, int width, int height) {
    m_width = width;
    m_height = height;

    if (m_textureId == 0) {
        glGenTextures(1, &m_textureId);
    }
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    // RPG RPG textures usually benefit from Pixel Art filter (Nearest)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Tiling support (MZ/MV spritesheets usually wrap)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Assume RGBA 32-bit for simple compatibility
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData.data());

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

void Texture::bind(uint32_t unit) {
    (void)unit;
    // glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
}

void Texture::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace urpg
