#include "sprite_batcher.h"
#include <algorithm>

namespace urpg {

SpriteBatcher::SpriteBatcher(uint32_t maxSprites) : m_maxSprites(maxSprites) {
}

SpriteBatcher::~SpriteBatcher() {
}

void SpriteBatcher::begin() {
    m_deferredQuads.clear();
    m_batches.clear();
}

void SpriteBatcher::submit(uint32_t textureId, float x, float y, float w, float h, float u1, float v1, float u2, float v2, float z, float r, float g, float b, float a) {
    DeferredQuad quad;
    quad.textureId = textureId;
    quad.x = x; quad.y = y; quad.w = w; quad.h = h;
    quad.u1 = u1; quad.v1 = v1; quad.u2 = u2; quad.v2 = v2;
    quad.z = z;
    quad.r = r; quad.g = g; quad.b = b; quad.a = a;
    
    m_deferredQuads.push_back(quad);
}

void SpriteBatcher::end() {
    // 1. Sort by Z (Front-to-Back for opaque, but here we use Painter's Back-to-Front for transparency)
    // Primary sort: Z-Order (Back-to-Front)
    // Secondary sort: Texture ID (Batching optimization)
    std::sort(m_deferredQuads.begin(), m_deferredQuads.end(), [](const DeferredQuad& a, const DeferredQuad& b) {
        if (a.z != b.z) return a.z < b.z;
        return a.textureId < b.textureId;
    });

    // 2. Generate optimized draw batches
    for (const auto& q : m_deferredQuads) {
        if (m_batches.empty() || m_batches.back().textureId != q.textureId) {
            m_batches.push_back({q.textureId, {}});
        }

        auto& v = m_batches.back().vertices;
        v.push_back({{q.x,     q.y,     q.z}, {q.u1, q.v1}, {q.r, q.g, q.b, q.a}});
        v.push_back({{q.x,     q.y + q.h, q.z}, {q.u1, q.v2}, {q.r, q.g, q.b, q.a}});
        v.push_back({{q.x + q.w, q.y,     q.z}, {q.u2, q.v1}, {q.r, q.g, q.b, q.a}});
        v.push_back({{q.x,     q.y + q.h, q.z}, {q.u1, q.v2}, {q.r, q.g, q.b, q.a}});
        v.push_back({{q.x + q.w, q.y + q.h, q.z}, {q.u2, q.v2}, {q.r, q.g, q.b, q.a}});
        v.push_back({{q.x + q.w, q.y,     q.z}, {q.u2, q.v1}, {q.r, q.g, q.b, q.a}});
    }
}

} // namespace urpg
