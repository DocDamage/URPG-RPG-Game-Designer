#include "engine/core/map/terrain_brush.h"

#include <algorithm>

namespace urpg::map {

namespace {

uint32_t nextSeed(uint32_t seed) {
    return (seed * 1664525u) + 1013904223u;
}

int32_t chooseWeighted(const std::vector<TerrainBrushWeightedTile>& weighted_tiles, uint32_t seed, int32_t fallback) {
    int32_t total = 0;
    for (const auto& tile : weighted_tiles) {
        total += std::max(0, tile.weight);
    }
    if (total <= 0) {
        return fallback;
    }
    int32_t pick = static_cast<int32_t>(seed % static_cast<uint32_t>(total));
    auto ordered = weighted_tiles;
    std::stable_sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) {
        return a.tile_id < b.tile_id;
    });
    for (const auto& tile : ordered) {
        const int32_t weight = std::max(0, tile.weight);
        if (pick < weight) {
            return tile.tile_id;
        }
        pick -= weight;
    }
    return fallback;
}

} // namespace

std::vector<TerrainBrushPoint> PreviewTerrainBrush(const TerrainBrush& brush,
                                                   int32_t origin_x,
                                                   int32_t origin_y,
                                                   uint32_t seed) {
    std::vector<TerrainBrushPoint> points;
    const int32_t width = std::max(1, brush.width);
    const int32_t height = std::max(1, brush.height);

    switch (brush.mode) {
    case TerrainBrushMode::Single:
        points.push_back({origin_x, origin_y, brush.tile_id});
        break;
    case TerrainBrushMode::Rectangle:
    case TerrainBrushMode::RandomWeighted:
    case TerrainBrushMode::Autotile:
        for (int32_t y = 0; y < height; ++y) {
            for (int32_t x = 0; x < width; ++x) {
                seed = nextSeed(seed);
                const int32_t tile = (brush.mode == TerrainBrushMode::RandomWeighted)
                    ? chooseWeighted(brush.weighted_tiles, seed, brush.tile_id)
                    : brush.tile_id;
                points.push_back({origin_x + x, origin_y + y, tile});
            }
        }
        break;
    case TerrainBrushMode::Line:
        for (int32_t x = 0; x < width; ++x) {
            points.push_back({origin_x + x, origin_y, brush.tile_id});
        }
        break;
    case TerrainBrushMode::Stamp:
        for (const auto& point : brush.stamp) {
            points.push_back({origin_x + point.x, origin_y + point.y, point.tile_id});
        }
        break;
    }

    std::stable_sort(points.begin(), points.end(), [](const auto& a, const auto& b) {
        if (a.y != b.y) {
            return a.y < b.y;
        }
        if (a.x != b.x) {
            return a.x < b.x;
        }
        return a.tile_id < b.tile_id;
    });
    return points;
}

} // namespace urpg::map
