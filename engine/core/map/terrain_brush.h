#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {

enum class TerrainBrushMode : uint8_t {
    Single = 0,
    Rectangle = 1,
    Line = 2,
    RandomWeighted = 3,
    Stamp = 4,
    Autotile = 5,
};

struct TerrainBrushPoint {
    int32_t x = 0;
    int32_t y = 0;
    int32_t tile_id = 0;
};

struct TerrainBrushWeightedTile {
    int32_t tile_id = 0;
    int32_t weight = 0;
};

struct TerrainBrush {
    TerrainBrushMode mode = TerrainBrushMode::Single;
    int32_t tile_id = 0;
    int32_t width = 1;
    int32_t height = 1;
    std::vector<TerrainBrushPoint> stamp;
    std::vector<TerrainBrushWeightedTile> weighted_tiles;
};

std::vector<TerrainBrushPoint> PreviewTerrainBrush(const TerrainBrush& brush,
                                                   int32_t origin_x,
                                                   int32_t origin_y,
                                                   uint32_t seed);

} // namespace urpg::map
