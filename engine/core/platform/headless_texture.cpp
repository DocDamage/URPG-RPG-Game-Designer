#include "gl_texture.h"

namespace urpg {

Texture::Texture() = default;

Texture::~Texture() = default;

bool Texture::loadFromMemory(const std::vector<uint8_t>& pixelData, int width, int height) {
    if (pixelData.empty() || width <= 0 || height <= 0) {
        m_width = 0;
        m_height = 0;
        return false;
    }

    m_width = width;
    m_height = height;
    return true;
}

void Texture::bind(uint32_t unit) {
    (void)unit;
}

void Texture::unbind() {}

} // namespace urpg
