#pragma once

#include <vector>
#include <memory>
#include <string>
#include "asset_loader.h"
#include "engine/core/sprite_batcher.h"

namespace urpg {

/**
 * @brief Represents a single layer of a tilemap.
 */
struct TileLayer {
    std::vector<int> data; // Tile IDs (indices into tileset)
};

/**
 * @brief Renders RPG Maker MZ/MV style tilemaps using grouped sprite batching.
 * Supports A-E tileset structures and layered rendering.
 */
class TilemapRenderer {
public:
    TilemapRenderer(int width, int height, int tileSize = 48);
    ~TilemapRenderer() = default;

    /**
     * @brief Set the tileset texture used for this tilemap.
     */
    void setTileset(const std::shared_ptr<Texture>& tileset);

    /**
     * @brief Add or update a layer in the tilemap.
     */
    void setLayer(int index, const std::vector<int>& data);

    /**
     * @brief Submits the entire tilemap to the SpriteBatcher.
     * @param batcher The batcher to submit vertices to.
     */
    void draw(SpriteBatcher& batcher);

private:
    int m_width;
    int m_height;
    int m_tileSize;
    std::shared_ptr<Texture> m_tileset;
    std::vector<TileLayer> m_layers;
};

} // namespace urpg
