#pragma once

#include "engine/core/map/terrain_brush.h"

#include <cstddef>
#include <vector>

namespace urpg::editor {

struct TerrainBrushPanelSnapshot {
    size_t preview_point_count = 0;
    int32_t first_tile_id = 0;
};

class TerrainBrushPanel {
public:
    void preview(const urpg::map::TerrainBrush& brush, int32_t x, int32_t y, uint32_t seed);
    void render();

    const TerrainBrushPanelSnapshot& snapshot() const { return snapshot_; }
    const std::vector<urpg::map::TerrainBrushPoint>& points() const { return points_; }
    bool hasRenderedFrame() const { return has_rendered_frame_; }

private:
    std::vector<urpg::map::TerrainBrushPoint> points_;
    TerrainBrushPanelSnapshot snapshot_{};
    bool has_rendered_frame_ = false;
};

} // namespace urpg::editor
