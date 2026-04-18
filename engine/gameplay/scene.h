#pragma once

#include "engine/core/ecs/world.h"
#include "engine/core/render/tilemap.h"
#include "engine/core/render/i_renderer.h"
#include <memory>

namespace urpg {

/**
 * @brief Manages the current active level/scene.
 */
class Scene {
public:
    Scene(int width, int height) : m_tileMap(width, height) {}

    TileMap& getTileMap() { return m_tileMap; }

    void render(IRenderer& renderer) {
        m_tileMap.render(renderer);
    }

private:
    TileMap m_tileMap;
};

} // namespace urpg
