#pragma once

#include "scene_manager.h"
#include "engine/core/input/input_core.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/assets/texture_registry.h"
#include <vector>
#include <cstdint>

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
    MapScene(const std::string& mapId, int width, int height) 
        : m_mapId(mapId), m_width(width), m_height(height) {
        m_tiles.resize(width * height, {0, true});
        
        // Initialize player movement component
        m_playerMovement.gridPos = {0, 0};
        m_playerMovement.lastGridPos = {0, 0};
        m_playerMovement.moveSpeed = 4.0f;
        m_playerMovement.isMoving = false;
    }

    SceneType getType() const override { return SceneType::MAP; }
    std::string getName() const override { return "Map_" + m_mapId; }

    void onUpdate(float dt) override {
        // 1. Process movement transitions
        MovementSystem::Update(m_playerMovement, dt);

        // 2. Submit render commands
        submitRenderCommands();
    }

    void handleInput(const urpg::input::InputCore& input) override {
        if (m_playerMovement.isMoving) return;

        Direction moveDir = Direction::Down;
        bool shouldMove = false;

        if (input.isActionActive(urpg::input::InputAction::MoveUp)) {
            moveDir = Direction::Up;
            shouldMove = true;
        } else if (input.isActionActive(urpg::input::InputAction::MoveDown)) {
            moveDir = Direction::Down;
            shouldMove = true;
        } else if (input.isActionActive(urpg::input::InputAction::MoveLeft)) {
            moveDir = Direction::Left;
            shouldMove = true;
        } else if (input.isActionActive(urpg::input::InputAction::MoveRight)) {
            moveDir = Direction::Right;
            shouldMove = true;
        }

        if (shouldMove) {
            auto collisionCheck = [this](int x, int y) {
                return this->checkCollision(x, y);
            };
            MovementSystem::TryMove(m_playerMovement, moveDir, collisionCheck);
        }
    }

    // Coordinate Authority
    bool checkCollision(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return true;
        return !m_tiles[y * m_width + x].isPassable;
    }

    void setTile(int x, int y, uint16_t id, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x] = {id, passable};
        }
    }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    const MovementComponent& getPlayerMovement() const { return m_playerMovement; }

private:
    /**
     * @brief Translates native map state into render commands for the backend.
     */
    void submitRenderCommands() {
        auto& layer = RenderLayer::getInstance();

        // 1. Submit Tile commands for the base map
        for (int y = 0; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                auto cmd = std::make_shared<TileCommand>();
                cmd->x = static_cast<float>(x * 48); // Standard 48x48 tile size
                cmd->y = static_cast<float>(y * 48);
                cmd->tilesetId = "base_tileset"; // Placeholder
                cmd->tileIndex = m_tiles[y * m_width + x].tileId;
                cmd->zOrder = 0;
                layer.submit(cmd);
            }
        }

        // 2. Submit Sprite command for the player actor
        auto playerCmd = std::make_shared<SpriteCommand>();
        
        // Calculate interpolated screen position based on movement progress
        float screenX = static_cast<float>(m_playerMovement.gridPos.x * 48);
        float screenY = static_cast<float>(m_playerMovement.gridPos.y * 48);

        if (m_playerMovement.isMoving) {
            float lastX = static_cast<float>(m_playerMovement.lastGridPos.x * 48);
            float lastY = static_cast<float>(m_playerMovement.lastGridPos.y * 48);
            
            screenX = lastX + (screenX - lastX) * m_playerMovement.moveProgress;
            screenY = lastY + (screenY - lastY) * m_playerMovement.moveProgress;
        }

        playerCmd->x = screenX;
        playerCmd->y = screenY;
        playerCmd->textureId = "hero_sprite";
        playerCmd->width = 48;
        playerCmd->height = 48;
        playerCmd->zOrder = 10; // Player above tiles
        layer.submit(playerCmd);
    }

    std::string m_mapId;
    int m_width;
    int m_height;
    std::vector<TileData> m_tiles;

    MovementComponent m_playerMovement;
};

} // namespace urpg::scene
