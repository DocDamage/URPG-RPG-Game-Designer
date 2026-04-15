#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace urpg {

class Texture {
public:
    Texture();
    ~Texture();

    bool loadFromMemory(const std::vector<uint8_t>& pixelData, int width, int height);
    void bind(uint32_t unit = 0);
    void unbind();

    uint32_t getId() const { return m_textureId; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    uint32_t m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
};

} // namespace urpg
