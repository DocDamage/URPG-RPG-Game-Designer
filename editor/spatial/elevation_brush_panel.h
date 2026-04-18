#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/engine_context.h"
#include <vector>
#include <string>
#include <algorithm>

namespace urpg::editor {

/**
 * @brief Editor tool for modifying ElevationGrid data.
 * Exposes SpatialMapOverlay logic to non-technical authors.
 */
class ElevationBrushPanel : public EditorPanel {
public:
    ElevationBrushPanel() : EditorPanel("Elevation Brush") {}

    void Render(const urpg::FrameContext& context) override {
        if (!m_visible) return;

        // Note: Real ImGui calls would go here.
        // This simulates the logic exposed to the UI.
    }

    /**
     * @brief Applies the elevation brush at a specific grid coordinate.
     */
    void ApplyBrush(uint32_t x, uint32_t y, int8_t level) {
        if (!m_targetOverlay) return;

        auto& grid = m_targetOverlay->elevation;
        if (x < grid.width && y < grid.height) {
            grid.levels[y * grid.width + x] = level;
            
            // Interpolate neighbors if brush size > 1
            if (m_brushSize > 1) {
                for (int dy = -m_brushSize; dy <= m_brushSize; ++dy) {
                    for (int dx = -m_brushSize; dx <= m_brushSize; ++dx) {
                        uint32_t nx = (uint32_t)((int)x + dx);
                        uint32_t ny = (uint32_t)((int)y + dy);
                        if (nx < grid.width && ny < grid.height) {
                            // Simple linear falloff or just direct set for now
                            grid.levels[ny * grid.width + nx] = level;
                        }
                    }
                }
            }
        }
    }

    void SetTarget(urpg::presentation::SpatialMapOverlay* overlay) {
        m_targetOverlay = overlay;
    }

private:
    urpg::presentation::SpatialMapOverlay* m_targetOverlay = nullptr;
    float m_brushHeight = 1.0f;
    int m_brushSize = 1;
};

} // namespace urpg::editor
