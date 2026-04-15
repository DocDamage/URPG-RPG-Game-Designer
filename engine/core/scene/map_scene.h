#pragma once

#include "scene_manager.h"
#include "engine/core/input/input_core.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/render/tilemap_renderer.h"
#include "engine/core/render/sprite_animator.h"
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace urpg::scene {

/**
 * @brief Represents a tile in the map.
 */
struct TileData {
    uint16_t tileId = 0;
    bool isPassable = true;
};

/**
 * @brief Native authority for Map data, coordinates, and collision.
 */
class MapScene : public GameScene {
public:
    MapScene(const std::string& mapId, int width, int height);
    virtual ~MapScene() = default;

    SceneType getType() const override { return SceneType::MAP; }
    std::string getName() const override { return "Map_" + m_mapId; }

    // Lifecycle hooks
    void onUpdate(float deltaTime) override;
    void handleInput(const urpg::input::InputCore& input) override;

    /**
     * @brief Draw the map using the provided batcher.
     */
    void draw(SpriteBatcher& batcher) override;

    /**
     * @brief Set map data for a specific layer.
     */
    void setLayerData(int layer, const std::vector<int>& data);

    /**
     * @brief Associate a tileset texture with this map.
     */
    void setTileset(const std::shared_ptr<Texture>& tileset);

    /**
     * @brief Setup the player's visual sprite.
     */
    void setPlayerCharacter(const std::string& name, int index);

    /**
     * @brief Manually override passability for a specific tile.
     */
    void setTilePassable(int x, int y, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x].isPassable = passable;
        }
    }

    void setTile(int x, int y, uint16_t tileId, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x].tileId = tileId;
            m_tiles[y * m_width + x].isPassable = passable;
        }
    }

    // Coordinate Authority
    bool checkCollision(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return true;
        return !m_tiles[y * m_width + x].isPassable;
    }

    MovementComponent& getPlayerMovement() { return m_playerMovement; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    std::string m_mapId;
    int m_width;
    int m_height;
    std::vector<TileData> m_tiles;
    
    // Components
    urpg::MovementComponent m_playerMovement;
    std::unique_ptr<TilemapRenderer> m_renderer;
    std::unique_ptr<SpriteAnimator> m_playerAnimator;
};

} // namespace urpg::scene
