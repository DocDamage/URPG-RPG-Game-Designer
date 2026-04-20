#include "editor/spatial/elevation_brush_panel.h"

#include <algorithm>

namespace urpg::editor {

void ElevationBrushPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void ElevationBrushPanel::ApplyBrush(uint32_t x, uint32_t y, int8_t level) {
    if (!m_targetOverlay) {
        return;
    }

    auto& grid = m_targetOverlay->elevation;
    if (x >= grid.width || y >= grid.height) {
        captureRenderSnapshot();
        return;
    }

    grid.levels[y * grid.width + x] = level;

    if (m_brushSize > 1) {
        for (int dy = -m_brushSize; dy <= m_brushSize; ++dy) {
            for (int dx = -m_brushSize; dx <= m_brushSize; ++dx) {
                const int nx = static_cast<int>(x) + dx;
                const int ny = static_cast<int>(y) + dy;
                if (nx < 0 || ny < 0) {
                    continue;
                }

                const auto cast_x = static_cast<uint32_t>(nx);
                const auto cast_y = static_cast<uint32_t>(ny);
                if (cast_x < grid.width && cast_y < grid.height) {
                    grid.levels[cast_y * grid.width + cast_x] = level;
                }
            }
        }
    }

    captureRenderSnapshot();
}

void ElevationBrushPanel::SetTarget(urpg::presentation::SpatialMapOverlay* overlay) {
    m_targetOverlay = overlay;
    captureRenderSnapshot();
}

void ElevationBrushPanel::SetBrushSize(int brush_size) {
    m_brushSize = std::max(1, brush_size);
    captureRenderSnapshot();
}

void ElevationBrushPanel::SetBrushHeight(float brush_height) {
    m_brushHeight = brush_height;
    captureRenderSnapshot();
}

void ElevationBrushPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.brush_size = m_brushSize;
    last_render_snapshot_.brush_height = m_brushHeight;
    if (m_targetOverlay == nullptr) {
        return;
    }

    last_render_snapshot_.has_target = true;
    last_render_snapshot_.grid_width = m_targetOverlay->elevation.width;
    last_render_snapshot_.grid_height = m_targetOverlay->elevation.height;
    if (!m_targetOverlay->elevation.levels.empty()) {
        last_render_snapshot_.sampled_level = m_targetOverlay->elevation.levels.front();
    }
}

} // namespace urpg::editor
