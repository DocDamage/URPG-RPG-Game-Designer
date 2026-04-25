#include "editor/spatial/terrain_brush_panel.h"

namespace urpg::editor {

void TerrainBrushPanel::preview(const urpg::map::TerrainBrush& brush, int32_t x, int32_t y, uint32_t seed) {
    points_ = urpg::map::PreviewTerrainBrush(brush, x, y, seed);
    snapshot_.preview_point_count = points_.size();
    snapshot_.first_tile_id = points_.empty() ? 0 : points_.front().tile_id;
}

void TerrainBrushPanel::render() {
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
