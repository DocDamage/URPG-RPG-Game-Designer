#include "map_scene.h"

namespace urpg::scene {

MapScene::MapScene(const std::string& mapId, int width, int height)
    : m_mapId(mapId), m_width(width), m_height(height) {
    m_tiles.resize(width * height, {0, true});
    m_renderer = std::make_unique<TilemapRenderer>(width, height);
    
    // Initialize player movement component
    m_playerMovement.gridPos = {0, 0};
    m_playerMovement.lastGridPos = {0, 0};
    m_playerMovement.moveSpeed = 4.0f;
    m_playerMovement.isMoving = false;
}

void MapScene::onUpdate(float deltaTime) {
    // 1. Process movement transitions
    urpg::MovementSystem::Update(m_playerMovement, deltaTime);

    // 2. Sync animator state to movement
    if (m_playerAnimator) {
        m_playerAnimator->setMoving(m_playerMovement.isMoving);
        m_playerAnimator->setDirection(m_playerMovement.direction);
        m_playerAnimator->update(deltaTime);
    }

    // 3. Keep RenderLayer in sync for scene/engine tests and headless render pipelines.
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    constexpr float kTileSize = 48.0f;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const auto& tile = m_tiles[static_cast<size_t>(y * m_width + x)];
            auto tileCmd = std::make_shared<urpg::TileCommand>();
            tileCmd->tilesetId = "default_tileset";
            tileCmd->tileIndex = tile.tileId;
            tileCmd->x = static_cast<float>(x) * kTileSize;
            tileCmd->y = static_cast<float>(y) * kTileSize;
            tileCmd->zOrder = 0;
            layer.submit(tileCmd);
        }
    }

    float playerX = static_cast<float>(m_playerMovement.gridPos.x) * kTileSize;
    float playerY = static_cast<float>(m_playerMovement.gridPos.y) * kTileSize;
    if (m_playerMovement.isMoving) {
        const float lastX = static_cast<float>(m_playerMovement.lastGridPos.x) * kTileSize;
        const float lastY = static_cast<float>(m_playerMovement.lastGridPos.y) * kTileSize;
        playerX = lastX + (playerX - lastX) * m_playerMovement.moveProgress;
        playerY = lastY + (playerY - lastY) * m_playerMovement.moveProgress;
    }

    auto playerCmd = std::make_shared<urpg::SpriteCommand>();
    playerCmd->textureId = "hero_sprite";
    playerCmd->x = playerX;
    playerCmd->y = playerY;
    playerCmd->width = 48;
    playerCmd->height = 48;
    playerCmd->zOrder = 1;
    layer.submit(playerCmd);
}

void MapScene::handleInput(const urpg::input::InputCore& input) {
    if (m_playerMovement.isMoving) return;

    urpg::Direction moveDir = urpg::Direction::Down;
    bool shouldMove = false;

    if (input.isActionActive(urpg::input::InputAction::MoveUp)) {
        moveDir = urpg::Direction::Up;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveDown)) {
        moveDir = urpg::Direction::Down;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveLeft)) {
        moveDir = urpg::Direction::Left;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveRight)) {
        moveDir = urpg::Direction::Right;
        shouldMove = true;
    }

    if (shouldMove) {
        auto collisionCheck = [this](int x, int y) {
            return this->checkCollision(x, y);
        };
        urpg::MovementSystem::TryMove(m_playerMovement, moveDir, collisionCheck);
    }
}

void MapScene::draw(SpriteBatcher& batcher) {
    if (m_renderer) {
        m_renderer->draw(batcher);
    }

    if (m_playerAnimator) {
        // Calculate smooth pixel position based on grid pos and move progress
        float tileSize = 48.0f;
        float drawX = m_playerMovement.gridPos.x * tileSize;
        float drawY = m_playerMovement.gridPos.y * tileSize;

        if (m_playerMovement.isMoving) {
            float lastX = m_playerMovement.lastGridPos.x * tileSize;
            float lastY = m_playerMovement.lastGridPos.y * tileSize;
            drawX = lastX + (drawX - lastX) * m_playerMovement.moveProgress;
            drawY = lastY + (drawY - lastY) * m_playerMovement.moveProgress;
        }
        
        // Character sprites in RM are 48x48 or taller. 
        // Sync Z-order with Y coordinate to achieve RM-style "Depth sorting"
        // Base layers are at Z=0.0 to 0.5. Characters are at Z=0.6 + normalized Y.
        float z = 0.6f + (drawY / (m_height * 48.0f)) * 0.1f;
        
        m_playerAnimator->draw(batcher, drawX, drawY, 48.0f, 48.0f, z);
    }
}

void MapScene::setLayerData(int layer, const std::vector<int>& data) {
    if (m_renderer) {
        m_renderer->setLayer(layer, data);
    }
}

void MapScene::setTileset(const std::shared_ptr<Texture>& tileset) {
    if (m_renderer) {
        m_renderer->setTileset(tileset);
    }
}

void MapScene::setPlayerCharacter(const std::string& name, int index) {
    auto texture = urpg::AssetLoader::loadTexture("img/characters/" + name + ".png");
    m_playerAnimator = std::make_unique<SpriteAnimator>(texture);
    // index * 3 is typical for character sheet offset but simplified here
}

} // namespace urpg::scene
