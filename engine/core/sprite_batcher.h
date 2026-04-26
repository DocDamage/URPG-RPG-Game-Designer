#pragma once

#include <cstdint>
#include <vector>

namespace urpg {

struct SpriteVertex {
    float position[3]; // x y z (Z for sorting)
    float uv[2];       // u v
    float color[4];    // r g b a
};

struct SpriteDrawData {
    uint32_t textureId;
    std::vector<SpriteVertex> vertices;
};

class SpriteBatcher {
  public:
    SpriteBatcher(uint32_t maxSprites = 2000);
    ~SpriteBatcher();

    void begin();
    void submit(uint32_t textureId, float x, float y, float w, float h, float u1, float v1, float u2, float v2,
                float z = 0.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
    void end();

    const std::vector<SpriteDrawData>& getBatches() const { return m_batches; }

  private:
    struct DeferredQuad {
        uint32_t textureId;
        float x, y, w, h, u1, v1, u2, v2, z, r, g, b, a;
    };

    uint32_t m_maxSprites;
    std::vector<DeferredQuad> m_deferredQuads;
    std::vector<SpriteDrawData> m_batches;
};

} // namespace urpg
