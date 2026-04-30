#include "editor/spatial/map_ability_binding_panel.h"

#include "engine/core/scene/map_scene.h"

#include <algorithm>
#include <cmath>

namespace urpg::editor {

namespace {

const char* ScopeName(urpg::scene::MapScene::InteractionBindingScope scope) {
    switch (scope) {
    case urpg::scene::MapScene::InteractionBindingScope::Global:
        return "global";
    case urpg::scene::MapScene::InteractionBindingScope::Tile:
        return "tile";
    case urpg::scene::MapScene::InteractionBindingScope::Prop:
        return "prop";
    case urpg::scene::MapScene::InteractionBindingScope::Region:
        return "region";
    }

    return "global";
}

MapAbilityBindingPanel::PaintedRegion NormalizeRegion(int start_x, int start_y, int end_x, int end_y) {
    MapAbilityBindingPanel::PaintedRegion region;
    region.min_x = std::min(start_x, end_x);
    region.min_y = std::min(start_y, end_y);
    region.max_x = std::max(start_x, end_x);
    region.max_y = std::max(start_y, end_y);
    return region;
}

bool RegionMatches(const MapAbilityBindingPanel::PaintedRegion& lhs, const MapAbilityBindingPanel::PaintedRegion& rhs) {
    return lhs.min_x == rhs.min_x && lhs.min_y == rhs.min_y && lhs.max_x == rhs.max_x && lhs.max_y == rhs.max_y;
}

std::optional<urpg::ability::AuthoredAbilityAsset>
LoadAssetForBinding(const std::filesystem::path& project_root,
                    const urpg::scene::MapScene::InteractionAbilityBinding& binding) {
    if (binding.asset_path.empty()) {
        return std::nullopt;
    }

    const std::filesystem::path absolute_path =
        project_root.empty() ? std::filesystem::path(binding.asset_path) : (project_root / binding.asset_path);
    return urpg::ability::loadAuthoredAbilityAssetFromFile(absolute_path);
}

std::optional<urpg::ability::AuthoredAbilityAsset> LoadAssetFromRelativePath(const std::filesystem::path& project_root,
                                                                             const std::string& asset_path) {
    if (asset_path.empty()) {
        return std::nullopt;
    }

    const std::filesystem::path absolute_path =
        project_root.empty() ? std::filesystem::path(asset_path) : (project_root / asset_path);
    return urpg::ability::loadAuthoredAbilityAssetFromFile(absolute_path);
}

template<typename Predicate>
std::optional<urpg::scene::MapScene::InteractionAbilityBinding> FindBindingCopy(const urpg::scene::MapScene* scene,
                                                                                Predicate&& predicate) {
    if (scene == nullptr) {
        return std::nullopt;
    }

    for (const auto& binding : scene->interactionAbilityBindings()) {
        if (predicate(binding)) {
            return binding;
        }
    }

    return std::nullopt;
}

} // namespace

void MapAbilityBindingPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void MapAbilityBindingPanel::SetTarget(urpg::scene::MapScene* scene) {
    m_target_scene = scene;
    captureRenderSnapshot();
}

void MapAbilityBindingPanel::SetSpatialTarget(urpg::presentation::SpatialMapOverlay* overlay) {
    m_target_overlay = overlay;
    captureRenderSnapshot();
}

bool MapAbilityBindingPanel::SetProjectRoot(const std::string& root_path) {
    const std::filesystem::path next_root = root_path;
    if (m_project_root == next_root) {
        return RefreshProjectAssets();
    }

    m_project_root = next_root;
    return RefreshProjectAssets();
}

bool MapAbilityBindingPanel::RefreshProjectAssets() {
    std::optional<std::string> previously_selected_path;
    if (m_selected_asset_index.has_value() && *m_selected_asset_index < m_assets.size()) {
        previously_selected_path = m_assets[*m_selected_asset_index].relative_path;
    }

    m_assets = m_project_root.empty() ? std::vector<urpg::ability::AuthoredAbilityAssetRecord>{}
                                      : urpg::ability::discoverAuthoredAbilityAssets(m_project_root);
    m_selected_asset_index.reset();

    if (previously_selected_path.has_value()) {
        for (size_t i = 0; i < m_assets.size(); ++i) {
            if (m_assets[i].relative_path == *previously_selected_path) {
                m_selected_asset_index = i;
                captureRenderSnapshot();
                return true;
            }
        }
    }

    if (!m_assets.empty()) {
        m_selected_asset_index = 0;
    }

    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SelectAsset(size_t index) {
    if (index >= m_assets.size()) {
        return false;
    }
    if (m_selected_asset_index.has_value() && *m_selected_asset_index == index) {
        return false;
    }

    m_selected_asset_index = index;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SelectAssetByAbilityId(const std::string& ability_id) {
    if (ability_id.empty()) {
        return false;
    }

    for (size_t i = 0; i < m_assets.size(); ++i) {
        if (m_assets[i].ability_id == ability_id) {
            return SelectAsset(i);
        }
    }

    return false;
}

bool MapAbilityBindingPanel::SetSelectedTriggerId(const std::string& trigger_id) {
    if (m_selected_trigger_id == trigger_id) {
        return false;
    }

    m_selected_trigger_id = trigger_id;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SetPlacementTile(int tile_x, int tile_y) {
    if (m_placement.tile_x == tile_x && m_placement.tile_y == tile_y) {
        return false;
    }

    m_placement.tile_x = tile_x;
    m_placement.tile_y = tile_y;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SetSelectedPropAssetId(const std::string& prop_asset_id) {
    if (m_placement.prop_asset_id == prop_asset_id) {
        return false;
    }

    m_placement.prop_instance_id.clear();
    m_placement.prop_asset_id = prop_asset_id;
    m_selected_prop_index.reset();
    m_placement.selected_prop_index = static_cast<size_t>(-1);
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SetActiveRegionBounds(int min_x, int min_y, int max_x, int max_y) {
    const PaintedRegion normalized = NormalizeRegion(min_x, min_y, max_x, max_y);
    if (m_placement.region_start_x == normalized.min_x && m_placement.region_start_y == normalized.min_y &&
        m_placement.region_end_x == normalized.max_x && m_placement.region_end_y == normalized.max_y) {
        return false;
    }

    m_placement.region_start_x = normalized.min_x;
    m_placement.region_start_y = normalized.min_y;
    m_placement.region_end_x = normalized.max_x;
    m_placement.region_end_y = normalized.max_y;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SelectPropHandle(size_t index) {
    if (m_target_overlay == nullptr || index >= m_target_overlay->props.size()) {
        return false;
    }
    if (m_selected_prop_index.has_value() && *m_selected_prop_index == index) {
        return false;
    }

    m_selected_prop_index = index;
    m_placement.selected_prop_index = index;
    m_placement.prop_instance_id = m_target_overlay->props[index].instanceId;
    m_placement.prop_asset_id = m_target_overlay->props[index].assetId;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToTileFromScreen(
    float screenX, float screenY, const PropPlacementPanel::ScreenProjectionSettings& settings) {
    if (m_target_overlay == nullptr) {
        return false;
    }

    float worldX = 0.0f;
    float worldY = 0.0f;
    float worldZ = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(*m_target_overlay, screenX, screenY, settings, worldX, worldY,
                                                      worldZ)) {
        return false;
    }

    SetPlacementTile(static_cast<int>(std::floor(worldX)), static_cast<int>(std::floor(worldZ)));
    return BindSelectedAbilityToPlacementTile();
}

bool MapAbilityBindingPanel::SelectPropFromScreen(float screenX, float screenY,
                                                  const PropPlacementPanel::ScreenProjectionSettings& settings,
                                                  float max_distance) {
    if (m_target_overlay == nullptr) {
        return false;
    }

    float worldX = 0.0f;
    float worldY = 0.0f;
    float worldZ = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(*m_target_overlay, screenX, screenY, settings, worldX, worldY,
                                                      worldZ)) {
        return false;
    }

    std::optional<size_t> best_prop_index;
    float best_distance_sq = max_distance * max_distance;
    for (size_t i = 0; i < m_target_overlay->props.size(); ++i) {
        const auto& prop = m_target_overlay->props[i];
        const float dx = prop.posX - worldX;
        const float dz = prop.posZ - worldZ;
        const float distance_sq = dx * dx + dz * dz;
        if (distance_sq <= best_distance_sq) {
            best_distance_sq = distance_sq;
            best_prop_index = i;
        }
    }

    if (!best_prop_index.has_value()) {
        return false;
    }

    return SelectPropHandle(*best_prop_index);
}

bool MapAbilityBindingPanel::BeginPaintRegionFromScreen(float screenX, float screenY,
                                                        const PropPlacementPanel::ScreenProjectionSettings& settings) {
    if (m_target_overlay == nullptr) {
        return false;
    }

    float worldX = 0.0f;
    float worldY = 0.0f;
    float worldZ = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(*m_target_overlay, screenX, screenY, settings, worldX, worldY,
                                                      worldZ)) {
        return false;
    }

    m_placement.region_start_x = static_cast<int>(std::floor(worldX));
    m_placement.region_start_y = static_cast<int>(std::floor(worldZ));
    m_placement.region_end_x = m_placement.region_start_x;
    m_placement.region_end_y = m_placement.region_start_y;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::UpdatePaintRegionFromScreen(float screenX, float screenY,
                                                         const PropPlacementPanel::ScreenProjectionSettings& settings) {
    if (m_target_overlay == nullptr) {
        return false;
    }

    float worldX = 0.0f;
    float worldY = 0.0f;
    float worldZ = 0.0f;
    if (!PropPlacementPanel::TryProjectScreenToGround(*m_target_overlay, screenX, screenY, settings, worldX, worldY,
                                                      worldZ)) {
        return false;
    }

    m_placement.region_end_x = static_cast<int>(std::floor(worldX));
    m_placement.region_end_y = static_cast<int>(std::floor(worldZ));
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::CommitPaintedRegion() {
    const PaintedRegion committed = NormalizeRegion(m_placement.region_start_x, m_placement.region_start_y,
                                                    m_placement.region_end_x, m_placement.region_end_y);

    for (size_t i = 0; i < m_placement.painted_regions.size(); ++i) {
        if (RegionMatches(m_placement.painted_regions[i], committed)) {
            m_selected_painted_region_index = i;
            for (size_t j = 0; j < m_placement.painted_regions.size(); ++j) {
                m_placement.painted_regions[j].selected = (i == j);
            }
            captureRenderSnapshot();
            return false;
        }
    }

    for (auto& region : m_placement.painted_regions) {
        region.selected = false;
    }

    m_placement.painted_regions.push_back(committed);
    m_selected_painted_region_index = m_placement.painted_regions.size() - 1;
    m_placement.painted_regions.back().selected = true;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::SelectPaintedRegion(size_t index) {
    if (index >= m_placement.painted_regions.size()) {
        return false;
    }
    if (m_selected_painted_region_index.has_value() && *m_selected_painted_region_index == index) {
        return false;
    }

    m_selected_painted_region_index = index;
    for (size_t i = 0; i < m_placement.painted_regions.size(); ++i) {
        m_placement.painted_regions[i].selected = (i == index);
    }
    const auto& region = m_placement.painted_regions[index];
    m_placement.region_start_x = region.min_x;
    m_placement.region_start_y = region.min_y;
    m_placement.region_end_x = region.max_x;
    m_placement.region_end_y = region.max_y;
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::ClearPaintedRegions() {
    if (m_placement.painted_regions.empty()) {
        return false;
    }

    m_placement.painted_regions.clear();
    m_selected_painted_region_index.reset();
    captureRenderSnapshot();
    return true;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToConfirmInteraction() {
    if (m_target_scene == nullptr || !m_selected_asset_index.has_value()) {
        return false;
    }

    const auto& record = m_assets[*m_selected_asset_index];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool bound = m_target_scene->bindInteractionAbility(m_selected_trigger_id, record.relative_path, *asset);
    captureRenderSnapshot();
    return bound;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToPlacementTile() {
    if (m_target_scene == nullptr || !m_selected_asset_index.has_value()) {
        return false;
    }

    const auto& record = m_assets[*m_selected_asset_index];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool bound = m_target_scene->bindTileInteractionAbility(m_selected_trigger_id, m_placement.tile_x,
                                                                  m_placement.tile_y, record.relative_path, *asset);
    captureRenderSnapshot();
    return bound;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToSelectedProp() {
    if (m_target_scene == nullptr || !m_selected_asset_index.has_value() || m_placement.prop_asset_id.empty()) {
        return false;
    }

    const auto& record = m_assets[*m_selected_asset_index];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool bound =
        m_placement.prop_instance_id.empty()
            ? m_target_scene->bindPropInteractionAbility(m_selected_trigger_id, m_placement.prop_asset_id,
                                                         record.relative_path, *asset)
            : m_target_scene->bindPropInstanceInteractionAbility(m_selected_trigger_id, m_placement.prop_instance_id,
                                                                 m_placement.prop_asset_id, record.relative_path,
                                                                 *asset);
    captureRenderSnapshot();
    return bound;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToPaintedRegion() {
    if (m_target_scene == nullptr || !m_selected_asset_index.has_value()) {
        return false;
    }

    const auto& record = m_assets[*m_selected_asset_index];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool bound = m_target_scene->bindRegionInteractionAbility(
        m_selected_trigger_id, m_placement.region_start_x, m_placement.region_start_y, m_placement.region_end_x,
        m_placement.region_end_y, record.relative_path, *asset);
    captureRenderSnapshot();
    return bound;
}

bool MapAbilityBindingPanel::BindSelectedAbilityToCommittedRegions() {
    if (m_target_scene == nullptr || !m_selected_asset_index.has_value() || m_placement.painted_regions.empty()) {
        return false;
    }

    const auto& record = m_assets[*m_selected_asset_index];
    const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(record.absolute_path);
    if (!asset.has_value()) {
        return false;
    }

    bool bound_any = false;
    for (const auto& region : m_placement.painted_regions) {
        bound_any =
            m_target_scene->bindRegionInteractionAbility(m_selected_trigger_id, region.min_x, region.min_y,
                                                         region.max_x, region.max_y, record.relative_path, *asset) ||
            bound_any;
    }

    captureRenderSnapshot();
    return bound_any;
}

bool MapAbilityBindingPanel::MoveTileBinding(int from_tile_x, int from_tile_y, int to_tile_x, int to_tile_y) {
    if (m_target_scene == nullptr || (from_tile_x == to_tile_x && from_tile_y == to_tile_y)) {
        return false;
    }

    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Tile ||
            binding.trigger_id != m_selected_trigger_id || binding.tile_x != from_tile_x ||
            binding.tile_y != from_tile_y) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindTileInteractionAbility(binding.trigger_id, from_tile_x, from_tile_y);
        const bool rebound = m_target_scene->bindTileInteractionAbility(binding.trigger_id, to_tile_x, to_tile_y,
                                                                        binding.asset_path, *asset);
        if (rebound) {
            SetPlacementTile(to_tile_x, to_tile_y);
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::MovePropBinding(const std::string& from_prop_asset_id,
                                             const std::string& to_prop_asset_id) {
    if (m_target_scene == nullptr || from_prop_asset_id.empty() || to_prop_asset_id.empty() ||
        from_prop_asset_id == to_prop_asset_id) {
        return false;
    }

    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Prop ||
            binding.trigger_id != m_selected_trigger_id || binding.prop_asset_id != from_prop_asset_id) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindPropInteractionAbility(binding.trigger_id, from_prop_asset_id);
        const bool rebound = m_target_scene->bindPropInteractionAbility(binding.trigger_id, to_prop_asset_id,
                                                                        binding.asset_path, *asset);
        if (rebound) {
            SetSelectedPropAssetId(to_prop_asset_id);
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::ResizeRegionBinding(int from_min_x, int from_min_y, int from_max_x, int from_max_y,
                                                 int to_min_x, int to_min_y, int to_max_x, int to_max_y) {
    if (m_target_scene == nullptr) {
        return false;
    }

    const PaintedRegion from_region = NormalizeRegion(from_min_x, from_min_y, from_max_x, from_max_y);
    const PaintedRegion to_region = NormalizeRegion(to_min_x, to_min_y, to_max_x, to_max_y);
    if (RegionMatches(from_region, to_region)) {
        return false;
    }

    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Region ||
            binding.trigger_id != m_selected_trigger_id || binding.region_min_x != from_region.min_x ||
            binding.region_min_y != from_region.min_y || binding.region_max_x != from_region.max_x ||
            binding.region_max_y != from_region.max_y) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindRegionInteractionAbility(binding.trigger_id, from_region.min_x, from_region.min_y,
                                                       from_region.max_x, from_region.max_y);
        const bool rebound =
            m_target_scene->bindRegionInteractionAbility(binding.trigger_id, to_region.min_x, to_region.min_y,
                                                         to_region.max_x, to_region.max_y, binding.asset_path, *asset);
        if (rebound) {
            SetActiveRegionBounds(to_region.min_x, to_region.min_y, to_region.max_x, to_region.max_y);
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::SwitchTileBindingTrigger(int tile_x, int tile_y, const std::string& from_trigger_id,
                                                      const std::string& to_trigger_id) {
    if (m_target_scene == nullptr || from_trigger_id.empty() || to_trigger_id.empty() ||
        from_trigger_id == to_trigger_id) {
        return false;
    }

    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Tile ||
            binding.trigger_id != from_trigger_id || binding.tile_x != tile_x || binding.tile_y != tile_y) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindTileInteractionAbility(from_trigger_id, tile_x, tile_y);
        const bool rebound =
            m_target_scene->bindTileInteractionAbility(to_trigger_id, tile_x, tile_y, binding.asset_path, *asset);
        if (rebound) {
            m_selected_trigger_id = to_trigger_id;
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::SwitchPropBindingTrigger(const std::string& prop_asset_id,
                                                      const std::string& from_trigger_id,
                                                      const std::string& to_trigger_id) {
    if (m_target_scene == nullptr || prop_asset_id.empty() || from_trigger_id.empty() || to_trigger_id.empty() ||
        from_trigger_id == to_trigger_id) {
        return false;
    }

    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Prop ||
            binding.trigger_id != from_trigger_id || binding.prop_asset_id != prop_asset_id) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindPropInteractionAbility(from_trigger_id, prop_asset_id);
        const bool rebound =
            m_target_scene->bindPropInteractionAbility(to_trigger_id, prop_asset_id, binding.asset_path, *asset);
        if (rebound) {
            m_selected_trigger_id = to_trigger_id;
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::SwitchRegionBindingTrigger(int min_x, int min_y, int max_x, int max_y,
                                                        const std::string& from_trigger_id,
                                                        const std::string& to_trigger_id) {
    if (m_target_scene == nullptr || from_trigger_id.empty() || to_trigger_id.empty() ||
        from_trigger_id == to_trigger_id) {
        return false;
    }

    const PaintedRegion region = NormalizeRegion(min_x, min_y, max_x, max_y);
    for (const auto& binding : m_target_scene->interactionAbilityBindings()) {
        if (binding.scope != urpg::scene::MapScene::InteractionBindingScope::Region ||
            binding.trigger_id != from_trigger_id || binding.region_min_x != region.min_x ||
            binding.region_min_y != region.min_y || binding.region_max_x != region.max_x ||
            binding.region_max_y != region.max_y) {
            continue;
        }

        const auto asset = LoadAssetForBinding(m_project_root, binding);
        if (!asset.has_value()) {
            return false;
        }

        m_target_scene->unbindRegionInteractionAbility(from_trigger_id, region.min_x, region.min_y, region.max_x,
                                                       region.max_y);
        const bool rebound = m_target_scene->bindRegionInteractionAbility(
            to_trigger_id, region.min_x, region.min_y, region.max_x, region.max_y, binding.asset_path, *asset);
        if (rebound) {
            m_selected_trigger_id = to_trigger_id;
        }
        captureRenderSnapshot();
        return rebound;
    }

    return false;
}

bool MapAbilityBindingPanel::ReplaceTileBindingAsset(int tile_x, int tile_y, const std::string& trigger_id,
                                                     const std::string& asset_path) {
    if (m_target_scene == nullptr || trigger_id.empty() || asset_path.empty()) {
        return false;
    }

    const auto asset = LoadAssetFromRelativePath(m_project_root, asset_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool rebound = m_target_scene->bindTileInteractionAbility(trigger_id, tile_x, tile_y, asset_path, *asset);
    captureRenderSnapshot();
    return rebound;
}

bool MapAbilityBindingPanel::ReplacePropBindingAsset(const std::string& prop_asset_id, const std::string& trigger_id,
                                                     const std::string& asset_path) {
    if (m_target_scene == nullptr || prop_asset_id.empty() || trigger_id.empty() || asset_path.empty()) {
        return false;
    }

    const auto asset = LoadAssetFromRelativePath(m_project_root, asset_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool rebound = m_target_scene->bindPropInteractionAbility(trigger_id, prop_asset_id, asset_path, *asset);
    captureRenderSnapshot();
    return rebound;
}

bool MapAbilityBindingPanel::ReplaceRegionBindingAsset(int min_x, int min_y, int max_x, int max_y,
                                                       const std::string& trigger_id, const std::string& asset_path) {
    if (m_target_scene == nullptr || trigger_id.empty() || asset_path.empty()) {
        return false;
    }

    const auto region = NormalizeRegion(min_x, min_y, max_x, max_y);
    const auto asset = LoadAssetFromRelativePath(m_project_root, asset_path);
    if (!asset.has_value()) {
        return false;
    }

    const bool rebound = m_target_scene->bindRegionInteractionAbility(trigger_id, region.min_x, region.min_y,
                                                                      region.max_x, region.max_y, asset_path, *asset);
    captureRenderSnapshot();
    return rebound;
}

bool MapAbilityBindingPanel::SwapTileBindingTriggers(int tile_x, int tile_y, const std::string& first_trigger_id,
                                                     const std::string& second_trigger_id) {
    if (m_target_scene == nullptr || first_trigger_id.empty() || second_trigger_id.empty() ||
        first_trigger_id == second_trigger_id) {
        return false;
    }

    const auto first_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Tile && binding.tile_x == tile_x &&
               binding.tile_y == tile_y && binding.trigger_id == first_trigger_id;
    });
    const auto second_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Tile && binding.tile_x == tile_x &&
               binding.tile_y == tile_y && binding.trigger_id == second_trigger_id;
    });
    if (!first_binding.has_value() || !second_binding.has_value()) {
        return false;
    }

    const auto first_asset = LoadAssetForBinding(m_project_root, *first_binding);
    const auto second_asset = LoadAssetForBinding(m_project_root, *second_binding);
    if (!first_asset.has_value() || !second_asset.has_value()) {
        return false;
    }

    m_target_scene->unbindTileInteractionAbility(first_trigger_id, tile_x, tile_y);
    m_target_scene->unbindTileInteractionAbility(second_trigger_id, tile_x, tile_y);
    const bool rebound_first = m_target_scene->bindTileInteractionAbility(second_trigger_id, tile_x, tile_y,
                                                                          first_binding->asset_path, *first_asset);
    const bool rebound_second = m_target_scene->bindTileInteractionAbility(first_trigger_id, tile_x, tile_y,
                                                                           second_binding->asset_path, *second_asset);
    captureRenderSnapshot();
    return rebound_first && rebound_second;
}

bool MapAbilityBindingPanel::SwapPropBindingTriggers(const std::string& prop_asset_id,
                                                     const std::string& first_trigger_id,
                                                     const std::string& second_trigger_id) {
    if (m_target_scene == nullptr || prop_asset_id.empty() || first_trigger_id.empty() || second_trigger_id.empty() ||
        first_trigger_id == second_trigger_id) {
        return false;
    }

    const auto first_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Prop &&
               binding.prop_asset_id == prop_asset_id && binding.trigger_id == first_trigger_id;
    });
    const auto second_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Prop &&
               binding.prop_asset_id == prop_asset_id && binding.trigger_id == second_trigger_id;
    });
    if (!first_binding.has_value() || !second_binding.has_value()) {
        return false;
    }

    const auto first_asset = LoadAssetForBinding(m_project_root, *first_binding);
    const auto second_asset = LoadAssetForBinding(m_project_root, *second_binding);
    if (!first_asset.has_value() || !second_asset.has_value()) {
        return false;
    }

    m_target_scene->unbindPropInteractionAbility(first_trigger_id, prop_asset_id);
    m_target_scene->unbindPropInteractionAbility(second_trigger_id, prop_asset_id);
    const bool rebound_first = m_target_scene->bindPropInteractionAbility(second_trigger_id, prop_asset_id,
                                                                          first_binding->asset_path, *first_asset);
    const bool rebound_second = m_target_scene->bindPropInteractionAbility(first_trigger_id, prop_asset_id,
                                                                           second_binding->asset_path, *second_asset);
    captureRenderSnapshot();
    return rebound_first && rebound_second;
}

bool MapAbilityBindingPanel::SwapRegionBindingTriggers(int min_x, int min_y, int max_x, int max_y,
                                                       const std::string& first_trigger_id,
                                                       const std::string& second_trigger_id) {
    if (m_target_scene == nullptr || first_trigger_id.empty() || second_trigger_id.empty() ||
        first_trigger_id == second_trigger_id) {
        return false;
    }

    const auto region = NormalizeRegion(min_x, min_y, max_x, max_y);
    const auto first_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Region &&
               binding.region_min_x == region.min_x && binding.region_min_y == region.min_y &&
               binding.region_max_x == region.max_x && binding.region_max_y == region.max_y &&
               binding.trigger_id == first_trigger_id;
    });
    const auto second_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Region &&
               binding.region_min_x == region.min_x && binding.region_min_y == region.min_y &&
               binding.region_max_x == region.max_x && binding.region_max_y == region.max_y &&
               binding.trigger_id == second_trigger_id;
    });
    if (!first_binding.has_value() || !second_binding.has_value()) {
        return false;
    }

    const auto first_asset = LoadAssetForBinding(m_project_root, *first_binding);
    const auto second_asset = LoadAssetForBinding(m_project_root, *second_binding);
    if (!first_asset.has_value() || !second_asset.has_value()) {
        return false;
    }

    m_target_scene->unbindRegionInteractionAbility(first_trigger_id, region.min_x, region.min_y, region.max_x,
                                                   region.max_y);
    m_target_scene->unbindRegionInteractionAbility(second_trigger_id, region.min_x, region.min_y, region.max_x,
                                                   region.max_y);
    const bool rebound_first =
        m_target_scene->bindRegionInteractionAbility(second_trigger_id, region.min_x, region.min_y, region.max_x,
                                                     region.max_y, first_binding->asset_path, *first_asset);
    const bool rebound_second =
        m_target_scene->bindRegionInteractionAbility(first_trigger_id, region.min_x, region.min_y, region.max_x,
                                                     region.max_y, second_binding->asset_path, *second_asset);
    captureRenderSnapshot();
    return rebound_first && rebound_second;
}

bool MapAbilityBindingPanel::SwapRegionBindingTriggersBetween(int first_min_x, int first_min_y, int first_max_x,
                                                              int first_max_y, const std::string& first_trigger_id,
                                                              int second_min_x, int second_min_y, int second_max_x,
                                                              int second_max_y, const std::string& second_trigger_id) {
    if (m_target_scene == nullptr || first_trigger_id.empty() || second_trigger_id.empty()) {
        return false;
    }

    const PaintedRegion first_region = NormalizeRegion(first_min_x, first_min_y, first_max_x, first_max_y);
    const PaintedRegion second_region = NormalizeRegion(second_min_x, second_min_y, second_max_x, second_max_y);
    const auto first_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Region &&
               binding.region_min_x == first_region.min_x && binding.region_min_y == first_region.min_y &&
               binding.region_max_x == first_region.max_x && binding.region_max_y == first_region.max_y &&
               binding.trigger_id == first_trigger_id;
    });
    const auto second_binding = FindBindingCopy(m_target_scene, [&](const auto& binding) {
        return binding.scope == urpg::scene::MapScene::InteractionBindingScope::Region &&
               binding.region_min_x == second_region.min_x && binding.region_min_y == second_region.min_y &&
               binding.region_max_x == second_region.max_x && binding.region_max_y == second_region.max_y &&
               binding.trigger_id == second_trigger_id;
    });
    if (!first_binding.has_value() || !second_binding.has_value()) {
        return false;
    }

    const auto first_asset = LoadAssetForBinding(m_project_root, *first_binding);
    const auto second_asset = LoadAssetForBinding(m_project_root, *second_binding);
    if (!first_asset.has_value() || !second_asset.has_value()) {
        return false;
    }

    m_target_scene->unbindRegionInteractionAbility(first_trigger_id, first_region.min_x, first_region.min_y,
                                                   first_region.max_x, first_region.max_y);
    m_target_scene->unbindRegionInteractionAbility(second_trigger_id, second_region.min_x, second_region.min_y,
                                                   second_region.max_x, second_region.max_y);
    const bool rebound_first = m_target_scene->bindRegionInteractionAbility(
        second_trigger_id, first_region.min_x, first_region.min_y, first_region.max_x, first_region.max_y,
        first_binding->asset_path, *first_asset);
    const bool rebound_second = m_target_scene->bindRegionInteractionAbility(
        first_trigger_id, second_region.min_x, second_region.min_y, second_region.max_x, second_region.max_y,
        second_binding->asset_path, *second_asset);
    captureRenderSnapshot();
    return rebound_first && rebound_second;
}

bool MapAbilityBindingPanel::RemoveTileBinding(int tile_x, int tile_y, const std::string& trigger_id) {
    if (m_target_scene == nullptr || trigger_id.empty()) {
        return false;
    }

    const bool removed = m_target_scene->unbindTileInteractionAbility(trigger_id, tile_x, tile_y);
    captureRenderSnapshot();
    return removed;
}

bool MapAbilityBindingPanel::RemovePropBinding(const std::string& prop_asset_id, const std::string& trigger_id) {
    if (m_target_scene == nullptr || prop_asset_id.empty() || trigger_id.empty()) {
        return false;
    }

    const bool removed = m_target_scene->unbindPropInteractionAbility(trigger_id, prop_asset_id);
    captureRenderSnapshot();
    return removed;
}

bool MapAbilityBindingPanel::RemoveRegionBinding(int min_x, int min_y, int max_x, int max_y,
                                                 const std::string& trigger_id) {
    if (m_target_scene == nullptr || trigger_id.empty()) {
        return false;
    }

    const bool removed = m_target_scene->unbindRegionInteractionAbility(trigger_id, min_x, min_y, max_x, max_y);
    captureRenderSnapshot();
    return removed;
}

bool MapAbilityBindingPanel::ActivateConfirmInteraction() {
    if (m_target_scene == nullptr) {
        return false;
    }

    const bool activated = m_target_scene->activateInteractionAbility(m_selected_trigger_id);
    captureRenderSnapshot();
    return activated;
}

bool MapAbilityBindingPanel::ActivatePlacementTileInteraction() {
    if (m_target_scene == nullptr) {
        return false;
    }

    const bool activated =
        m_target_scene->activateInteractionAbilityAtTile(m_selected_trigger_id, m_placement.tile_x, m_placement.tile_y);
    captureRenderSnapshot();
    return activated;
}

bool MapAbilityBindingPanel::ActivateSelectedPropInteraction() {
    if (m_target_scene == nullptr || m_placement.prop_asset_id.empty()) {
        return false;
    }

    const bool activated =
        m_placement.prop_instance_id.empty()
            ? m_target_scene->activateInteractionAbilityForProp(m_selected_trigger_id, m_placement.prop_asset_id)
            : m_target_scene->activateInteractionAbilityForPropInstance(
                  m_selected_trigger_id, m_placement.prop_instance_id, m_placement.prop_asset_id);
    captureRenderSnapshot();
    return activated;
}

void MapAbilityBindingPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.project_root = m_project_root.generic_string();
    last_render_snapshot_.canonical_directory =
        m_project_root.empty() ? "" : urpg::ability::canonicalAbilityContentDirectory(m_project_root).generic_string();
    last_render_snapshot_.available_asset_count = m_assets.size();
    last_render_snapshot_.selected_trigger_id = m_selected_trigger_id;
    last_render_snapshot_.placement = m_placement;
    last_render_snapshot_.has_target_overlay = (m_target_overlay != nullptr);

    for (size_t i = 0; i < m_assets.size(); ++i) {
        const auto& asset = m_assets[i];
        const bool selected = m_selected_asset_index.has_value() && *m_selected_asset_index == i;
        last_render_snapshot_.assets.push_back({asset.relative_path, asset.ability_id, selected});
        if (selected) {
            last_render_snapshot_.selected_asset_path = asset.relative_path;
            last_render_snapshot_.selected_ability_id = asset.ability_id;
        }
    }

    if (m_target_overlay != nullptr) {
        for (size_t i = 0; i < m_target_overlay->props.size(); ++i) {
            const auto& prop = m_target_overlay->props[i];
            PropHandleEntry handle;
            handle.prop_index = i;
            handle.instance_id = prop.instanceId;
            handle.asset_id = prop.assetId;
            handle.world_x = prop.posX;
            handle.world_y = prop.posY;
            handle.world_z = prop.posZ;
            handle.tile_x = static_cast<int>(std::floor(prop.posX));
            handle.tile_y = static_cast<int>(std::floor(prop.posZ));
            handle.selected = m_selected_prop_index.has_value() && *m_selected_prop_index == i;
            last_render_snapshot_.prop_handles.push_back(std::move(handle));
        }

        TileOverlayEntry placement_tile;
        placement_tile.tile_x = m_placement.tile_x;
        placement_tile.tile_y = m_placement.tile_y;
        placement_tile.trigger_id = m_selected_trigger_id;
        placement_tile.ability_id = last_render_snapshot_.selected_ability_id;
        placement_tile.source = "placement";
        placement_tile.selected = true;
        placement_tile.pending = true;
        last_render_snapshot_.tile_overlays.push_back(std::move(placement_tile));

        RegionOverlayEntry active_region;
        active_region.min_x = std::min(m_placement.region_start_x, m_placement.region_end_x);
        active_region.min_y = std::min(m_placement.region_start_y, m_placement.region_end_y);
        active_region.max_x = std::max(m_placement.region_start_x, m_placement.region_end_x);
        active_region.max_y = std::max(m_placement.region_start_y, m_placement.region_end_y);
        active_region.trigger_id = m_selected_trigger_id;
        active_region.ability_id = last_render_snapshot_.selected_ability_id;
        active_region.source = "active_paint";
        active_region.selected = true;
        active_region.pending = true;
        last_render_snapshot_.region_overlays.push_back(std::move(active_region));

        for (size_t i = 0; i < m_placement.painted_regions.size(); ++i) {
            const auto& region = m_placement.painted_regions[i];
            RegionOverlayEntry overlay_region;
            overlay_region.min_x = region.min_x;
            overlay_region.min_y = region.min_y;
            overlay_region.max_x = region.max_x;
            overlay_region.max_y = region.max_y;
            overlay_region.trigger_id = m_selected_trigger_id;
            overlay_region.ability_id = last_render_snapshot_.selected_ability_id;
            overlay_region.source = "painted_region";
            overlay_region.selected = region.selected;
            overlay_region.pending = true;
            last_render_snapshot_.region_overlays.push_back(std::move(overlay_region));
        }
    }

    if (m_target_scene == nullptr) {
        last_render_snapshot_.prop_handle_count = last_render_snapshot_.prop_handles.size();
        last_render_snapshot_.tile_overlay_count = last_render_snapshot_.tile_overlays.size();
        last_render_snapshot_.region_overlay_count = last_render_snapshot_.region_overlays.size();
        return;
    }

    last_render_snapshot_.has_target_scene = true;
    const auto& bindings = m_target_scene->interactionAbilityBindings();
    last_render_snapshot_.binding_count = bindings.size();
    for (const auto& binding : bindings) {
        last_render_snapshot_.bindings.push_back({
            ScopeName(binding.scope),
            binding.trigger_id,
            binding.asset_path,
            binding.ability_id,
            binding.tile_x,
            binding.tile_y,
            binding.region_min_x,
            binding.region_min_y,
            binding.region_max_x,
            binding.region_max_y,
            binding.prop_instance_id,
            binding.prop_asset_id,
        });

        switch (binding.scope) {
        case urpg::scene::MapScene::InteractionBindingScope::Tile: {
            TileOverlayEntry tile_overlay;
            tile_overlay.tile_x = binding.tile_x;
            tile_overlay.tile_y = binding.tile_y;
            tile_overlay.trigger_id = binding.trigger_id;
            tile_overlay.ability_id = binding.ability_id;
            tile_overlay.source = "binding";
            tile_overlay.selected = (binding.tile_x == m_placement.tile_x && binding.tile_y == m_placement.tile_y);
            tile_overlay.pending = false;
            last_render_snapshot_.tile_overlays.push_back(std::move(tile_overlay));
            break;
        }
        case urpg::scene::MapScene::InteractionBindingScope::Region: {
            RegionOverlayEntry region_overlay;
            region_overlay.min_x = binding.region_min_x;
            region_overlay.min_y = binding.region_min_y;
            region_overlay.max_x = binding.region_max_x;
            region_overlay.max_y = binding.region_max_y;
            region_overlay.trigger_id = binding.trigger_id;
            region_overlay.ability_id = binding.ability_id;
            region_overlay.source = "binding";
            region_overlay.selected =
                region_overlay.min_x == std::min(m_placement.region_start_x, m_placement.region_end_x) &&
                region_overlay.min_y == std::min(m_placement.region_start_y, m_placement.region_end_y) &&
                region_overlay.max_x == std::max(m_placement.region_start_x, m_placement.region_end_x) &&
                region_overlay.max_y == std::max(m_placement.region_start_y, m_placement.region_end_y);
            region_overlay.pending = false;
            last_render_snapshot_.region_overlays.push_back(std::move(region_overlay));
            break;
        }
        case urpg::scene::MapScene::InteractionBindingScope::Prop:
            break;
        case urpg::scene::MapScene::InteractionBindingScope::Global:
            break;
        }
    }

    for (auto& handle : last_render_snapshot_.prop_handles) {
        for (const auto& binding : bindings) {
            if (binding.scope == urpg::scene::MapScene::InteractionBindingScope::Prop &&
                ((!binding.prop_instance_id.empty() && binding.prop_instance_id == handle.instance_id) ||
                 (binding.prop_instance_id.empty() && binding.prop_asset_id == handle.asset_id))) {
                handle.has_binding = true;
                break;
            }
        }
    }

    const auto& runtime = m_target_scene->playerAbilitySystem();
    last_render_snapshot_.runtime_ability_count = runtime.getAbilities().size();
    const auto& history = runtime.getAbilityExecutionHistory();
    if (!history.empty()) {
        last_render_snapshot_.latest_ability_id = history.back().ability_id;
        last_render_snapshot_.latest_outcome = history.back().outcome;
    }

    last_render_snapshot_.prop_handle_count = last_render_snapshot_.prop_handles.size();
    last_render_snapshot_.tile_overlay_count = last_render_snapshot_.tile_overlays.size();
    last_render_snapshot_.region_overlay_count = last_render_snapshot_.region_overlays.size();
}

} // namespace urpg::editor
