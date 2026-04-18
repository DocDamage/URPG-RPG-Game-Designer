#pragma once

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace urpg {

/**
 * @brief Represents collision flags for a single tile.
 * Matches RPG Maker MZ flag bits:
 * 0x01: Impassable Down
 * 0x02: Impassable Left
 * 0x04: Impassable Right
 * 0x08: Impassable Up
 * 0x10: Bush
 * 0x20: Counter
 * 0x40: Ladder
 */
enum TileFlag : uint16_t {
    Passable = 0,
    ImpassableDown = 1 << 0,
    ImpassableLeft = 1 << 1,
    ImpassableRight = 1 << 2,
    ImpassableUp = 1 << 3,
    Bush = 1 << 4,
    Counter = 1 << 5,
    Ladder = 1 << 6,
    FullImpassable = 0x0F
};

/**
 * @brief Stores flags for all tiles in a specific tileset.
 */
struct TilesetData {
    int32_t id = 0;
    std::string name;
    std::vector<uint16_t> flags; // Index is TileID
};

/**
 * @brief Authoritative registry for tileset collision data.
 */
class TilesetRegistry {
public:
    static TilesetRegistry& instance() {
        static TilesetRegistry inst;
        return inst;
    }

    void registerTileset(const TilesetData& data) {
        m_tilesets[data.id] = data;
    }

    const TilesetData* getTileset(int32_t id) const {
        auto it = m_tilesets.find(id);
        return (it != m_tilesets.end()) ? &it->second : nullptr;
    }

    bool isTilePassable(int32_t tilesetId, uint16_t tileId) const {
        const auto* ts = getTileset(tilesetId);
        if (!ts || tileId >= ts->flags.size()) return true;
        return (ts->flags[tileId] & FullImpassable) == 0;
    }

private:
    TilesetRegistry() = default;
    std::unordered_map<int32_t, TilesetData> m_tilesets;
};

} // namespace urpg
