#pragma once

#include "engine/core/render/i_renderer.h"
#include <iostream>

namespace urpg {

/**
 * @brief Logic for managing world tiles (Standard RPG 2D grid).
 */
struct Tile {
    int textureIndex;
    bool walkable;
};

class TileMap {
public:
    TileMap(int width, int height) : m_width(width), m_height(height) {
        m_tiles.resize(width * height, {0, true});
    }

    void setTile(int x, int y, const Tile& tile) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x] = tile;
        }
    }

    const Tile& getTile(int x, int y) const {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            return m_tiles[y * m_width + x];
        }
        static Tile empty = { -1, false };
        return empty;
    }

    void render(IRenderer& renderer) {
        const int tileSize = 32;
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                const Tile& tile = m_tiles[y * m_width + x];
                if (tile.textureIndex >= 0) {
                    renderer.drawSprite("tileset", 
                                        tile.textureIndex * tileSize, 0, tileSize, tileSize,
                                        static_cast<float>(x * tileSize), static_cast<float>(y * tileSize), 
                                        static_cast<float>(tileSize), static_cast<float>(tileSize));
                }
            }
        }
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    int m_width, m_height;
    std::vector<Tile> m_tiles;
};

} // namespace urpg
