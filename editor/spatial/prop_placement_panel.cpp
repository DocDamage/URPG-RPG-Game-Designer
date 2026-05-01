#include "editor/spatial/prop_placement_panel.h"

#include <algorithm>

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
    m_selectedProjectAssetId.clear();
    captureRenderSnapshot();
}

void PropPlacementPanel::SetProjectAssetOptions(std::vector<ProjectAssetOption> options) {
    m_projectAssetOptions.clear();
    for (auto& option : options) {
        option.targeted_for_level_builder =
            std::find(option.picker_targets.begin(), option.picker_targets.end(), "level_builder") !=
            option.picker_targets.end();
        if (option.targeted_for_level_builder) {
            m_projectAssetOptions.push_back(std::move(option));
        }
    }
    if (!m_selectedProjectAssetId.empty()) {
        const auto selected = std::find_if(
            m_projectAssetOptions.begin(), m_projectAssetOptions.end(), [&](const auto& option) {
                return option.asset_id == m_selectedProjectAssetId;
            });
        if (selected == m_projectAssetOptions.end()) {
            m_selectedProjectAssetId.clear();
        }
    }
    captureRenderSnapshot();
}

bool PropPlacementPanel::SelectProjectAsset(const std::string& project_path) {
    const auto found = std::find_if(m_projectAssetOptions.begin(), m_projectAssetOptions.end(), [&](const auto& option) {
        return option.project_path == project_path;
    });
    if (found == m_projectAssetOptions.end()) {
        return false;
    }
    m_selectedAssetId = found->project_path;
    m_selectedProjectAssetId = found->asset_id;
    captureRenderSnapshot();
    return true;
}

void PropPlacementPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.selected_asset_id = m_selectedAssetId;
    last_render_snapshot_.selected_project_asset_id = m_selectedProjectAssetId;
    last_render_snapshot_.project_asset_options = m_projectAssetOptions;
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
