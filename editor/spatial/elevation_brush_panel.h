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
    struct RenderSnapshot {
        bool visible = true;
        bool has_target = false;
        uint32_t grid_width = 0;
        uint32_t grid_height = 0;
        int brush_size = 1;
        float brush_height = 1.0f;
        int8_t sampled_level = 0;
    };

    ElevationBrushPanel() : EditorPanel("Elevation Brush") {}

    void Render(const urpg::FrameContext& context) override;

    /**
     * @brief Applies the elevation brush at a specific grid coordinate.
     */
    void ApplyBrush(uint32_t x, uint32_t y, int8_t level);

    void SetTarget(urpg::presentation::SpatialMapOverlay* overlay);
    void SetBrushSize(int brush_size);
    void SetBrushHeight(float brush_height);

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    void captureRenderSnapshot();

    urpg::presentation::SpatialMapOverlay* m_targetOverlay = nullptr;
    float m_brushHeight = 1.0f;
    int m_brushSize = 1;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
