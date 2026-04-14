#pragma once

#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/tileset_registry.h"
#include "runtimes/compat_js/data_manager.h"
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
        
        // In a real implementation, we would call dm.loadMapData(mapId) 
        // and then extract width, height, and tile data from the MZ JSON structure.
        
        // Mocking metadata extraction from DataManager
        int width = 20; 
        int height = 15;
        int tilesetId = 1; // Assume map uses tileset 1
        
        auto nativeMap = std::make_unique<urpg::scene::MapScene>(std::to_string(mapId), width, height);
        
        // Translation Logic:
        // MZ maps use 6 layers of tile IDs. We would map them to native TileData.
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Layer 0 is usually the background/floor (A1-A5)
                // We check the TilesetRegistry to see if this tile ID is passable.
                // For the mock, we assume tile ID 0 is floor and tile ID 1 is wall.
                uint16_t tileId = (x == 0 || x == width-1 || y == 0 || y == height-1) ? 1 : 0;
                
                bool passable = TilesetRegistry::instance().isTilePassable(tilesetId, tileId);
                nativeMap->setTile(x, y, tileId, passable);
            }
        }
        
        return nativeMap;
    }
};

} // namespace urpg
