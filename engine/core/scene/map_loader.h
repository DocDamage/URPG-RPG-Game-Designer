#pragma once

#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/tileset_registry.h"
#include "runtimes/compat_js/data_manager.h"
#include <algorithm>
#include <memory>
#include <string>

namespace urpg {

/**
 * @brief Bridge between the compat DataManager (MZ JSON data) and Native MapScene.
 */
class MapLoader {
  public:
    /**
     * @brief Translates a compat map data object into a native MapScene.
     * @param mapId The ID of the map to load.
     * @return A unique_ptr to the newly created MapScene.
     */
    static std::unique_ptr<urpg::scene::MapScene> LoadToNative(int32_t mapId) {
        auto& dm = urpg::compat::DataManager::instance();

        // 1. Ensure DataManager is synchronized with the requested map ID.
        if (!dm.loadMapData(mapId)) {
            return nullptr;
        }

        const auto* mapData = dm.getCurrentMap();
        if (!mapData)
            return nullptr;

        int width = mapData->width;
        int height = mapData->height;
        int tilesetId = mapData->tilesetId;

        auto nativeMap = std::make_unique<urpg::scene::MapScene>(std::to_string(mapId), width, height);

        // 2. Resolve Tileset Collision
        const auto* compatTileset = dm.getTileset(tilesetId);
        if (compatTileset) {
            // Synchronize Native TilesetRegistry
            TilesetData nativeTileset;
            nativeTileset.id = compatTileset->id;
            nativeTileset.name = compatTileset->name;
            for (auto flag : compatTileset->flags) {
                nativeTileset.flags.push_back(static_cast<uint16_t>(flag));
            }
            TilesetRegistry::instance().registerTileset(nativeTileset);
        }

        // 3. Move MZ Layers into Native Map
        for (size_t layerIdx = 0; layerIdx < mapData->data.size(); ++layerIdx) {
            nativeMap->setLayerData(static_cast<int>(layerIdx), mapData->data[layerIdx]);
        }

        // 4. Calculate Passability (simplified logic: check layer 0-3 for collision flags)
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                const size_t tileIndex = static_cast<size_t>(y * width + x);
                bool isPassable = true;
                const size_t collisionLayerCount = std::min<size_t>(4, mapData->data.size());

                // RPG Maker priority: top-down check
                for (size_t offset = 0; offset < collisionLayerCount; ++offset) {
                    const size_t layerIdx = collisionLayerCount - 1 - offset;
                    const int tileId = mapData->data[layerIdx][tileIndex];
                    if (tileId != 0 && !TilesetRegistry::instance().isTilePassable(tilesetId, tileId)) {
                        isPassable = false;
                        break;
                    }
                }

                // We set the base passability on the MapScene's collision grid
                nativeMap->setTilePassable(x, y, isPassable);
            }
        }

        // 5. Load Player Start (Sync from DataManager)
        [[maybe_unused]] const int startX = dm.getStartX();
        [[maybe_unused]] const int startY = dm.getStartY();
        // Here we would sync actor 1 character graphics
        const auto* actor = dm.getActor(dm.getStartPartyMember(0));
        if (actor) {
            nativeMap->setPlayerCharacter(actor->characterName, actor->characterIndex);
        }

        return nativeMap;
    }
};

} // namespace urpg
