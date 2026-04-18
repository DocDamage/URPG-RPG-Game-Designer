#include "tilemap_renderer.h"

namespace urpg {

TilemapRenderer::TilemapRenderer(int width, int height, int tileSize)
    : m_width(width), m_height(height), m_tileSize(tileSize) {
}

void TilemapRenderer::setTileset(const std::shared_ptr<Texture>& tileset) {
    m_tileset = tileset;
}

void TilemapRenderer::setLayer(int index, const std::vector<int>& data) {
    if (index >= (int)m_layers.size()) {
        m_layers.resize(index + 1);
    }
    m_layers[index].data = data;
}

void TilemapRenderer::draw(SpriteBatcher& batcher) {
    if (!m_tileset || m_layers.empty()) return;

    uint32_t texID = m_tileset->getId();
    int texW = m_tileset->getWidth();
    int texH = m_tileset->getHeight();

    // Calculate how many tiles wide the tileset is
    int tilesetWidthInTiles = texW / m_tileSize;

    for (int layerIdx = 0; layerIdx < (int)m_layers.size(); ++layerIdx) {
        const auto& layer = m_layers[layerIdx];
        if (layer.data.size() < (size_t)(m_width * m_height)) continue;

        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                int tileId = layer.data[y * m_width + x];
                if (tileId <= 0) continue; 

                int srcX = (tileId % tilesetWidthInTiles) * m_tileSize;
                int srcY = (tileId / tilesetWidthInTiles) * m_tileSize;

                float u1 = (float)srcX / texW;
                float v1 = (float)srcY / texH;
                float u2 = (float)(srcX + m_tileSize) / texW;
                float v2 = (float)(srcY + m_tileSize) / texH;

                // Use layer index for Z sorting (A tiles at 0.0, decorations at 0.1, etc.)
                float z = (float)layerIdx * 0.1f;

                batcher.submit(texID, 
                              (float)x * m_tileSize, (float)y * m_tileSize, 
                              (float)m_tileSize, (float)m_tileSize, 
                              u1, v1, u2, v2, z);
            }
        }
    }
}

} // namespace urpg
