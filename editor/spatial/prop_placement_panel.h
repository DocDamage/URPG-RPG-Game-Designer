#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/engine_context.h"
#include <vector>
#include <string>

namespace urpg::editor {

/**
 * @brief Editor tool for 3D Prop Placement.
 * Exposes SpatialMapOverlay logic to non-technical authors.
 */
class PropPlacementPanel : public EditorPanel {
public:
    PropPlacementPanel() : EditorPanel("Prop Placement") {}

    void Render(const urpg::FrameContext& context) override {
        if (!m_visible) return;

        // Note: Real ImGui calls would go here.
        // This simulates the logic exposed to the UI.
    }

    void AddProp(const std::string& assetId, float x, float y, float z) {
        if (m_targetOverlay) {
            m_targetOverlay->props.push_back({assetId, x, y, z, 0.0f, 1.0f});
        }
    }

    void SetTarget(urpg::presentation::SpatialMapOverlay* overlay) {
        m_targetOverlay = overlay;
    }

private:
    urpg::presentation::SpatialMapOverlay* m_targetOverlay = nullptr;
    std::string m_selectedAssetId = "tree_01";
};

} // namespace urpg::editor
