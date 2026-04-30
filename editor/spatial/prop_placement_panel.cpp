#include "editor/spatial/prop_placement_panel.h"

namespace urpg::editor {

void PropPlacementPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void PropPlacementPanel::AddProp(const std::string& assetId, float x, float y, float z) {
    if (m_targetOverlay == nullptr) {
        return;
    }

    int32_t assetInstanceIndex = 0;
    for (const auto& prop : m_targetOverlay->props) {
        if (prop.assetId == assetId) {
            ++assetInstanceIndex;
        }
    }

    const std::string instanceId = m_targetOverlay->mapId + ":" + assetId + ":" + std::to_string(assetInstanceIndex);
    m_targetOverlay->props.push_back({instanceId, assetId, x, y, z, 0.0f, 1.0f});
    captureRenderSnapshot();
}

void PropPlacementPanel::SetTarget(urpg::presentation::SpatialMapOverlay* overlay) {
    m_targetOverlay = overlay;
    captureRenderSnapshot();
}

void PropPlacementPanel::SetSelectedAssetId(const std::string& asset_id) {
    m_selectedAssetId = asset_id;
    captureRenderSnapshot();
}

void PropPlacementPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.selected_asset_id = m_selectedAssetId;
    if (m_targetOverlay == nullptr) {
        return;
    }

    last_render_snapshot_.has_target = true;
    last_render_snapshot_.prop_count = m_targetOverlay->props.size();
    if (!m_targetOverlay->props.empty()) {
        const auto& prop = m_targetOverlay->props.back();
        last_render_snapshot_.last_added_asset_id = prop.assetId;
        last_render_snapshot_.last_added_x = prop.posX;
        last_render_snapshot_.last_added_y = prop.posY;
        last_render_snapshot_.last_added_z = prop.posZ;
    }
}

} // namespace urpg::editor
